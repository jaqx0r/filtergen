#include "include.h"

#include <dirent.h>
#include <errno.h>
#include <glob.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "error.h"
#include "input/filtergen/parser.h"
#include "input/filtergen/scanner.h"

/* include a file or directory */
void include_file(YYLTYPE *yylloc, const char *name) {
  struct stat st;
  struct dirent **namelist;
  char *fn;
  int n;

  struct sourceposition *pos;

  pos = make_sourcepos(yylloc);

  if (stat(name, &st)) {
    if (errno == ENOENT &&
        (index(name, '*') != NULL || index(name, '?') != NULL ||
         index(name, '[') != NULL)) {
      /* Globbing fiesta! */
      glob_t glob_buf;
      if (glob(name, 0, NULL, &glob_buf) != 0) {
        filter_error(pos, "failed to glob \"%s\": %s", name, strerror(errno));
      } else {
        /* We go through the list of files backwards, because
         * step_into_include_file() creates a stack of all the
         * files processed and then works on them in a LIFO
         * fashion -- which would make all of our rules files
         * go backwards.  Since I can't wrap my head around
         * why that is, exactly, I'm hacking it up with
         * this instead.  Fixination appreciated.
         */
        for (n = glob_buf.gl_pathc - 1; n >= 0; n--) {
          if (stat(glob_buf.gl_pathv[n], &st)) {
            filter_error(pos, "stat failed on globbed \"%s\": %s",
                         glob_buf.gl_pathv[n], strerror(errno));
          } else if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) {
            if (sourcefile_push(pos, glob_buf.gl_pathv[n])) {
              filtergen_in = current_srcfile->f;
              filtergen_lineno = current_srcfile->lineno;
              // yycolumn = current_srcfile->column;
              if (!filtergen_in) {
                filter_error(pos, "failed to open %s", glob_buf.gl_pathv[n]);
              } else {
                filtergen_push_buffer_state(
                    filtergen__create_buffer(filtergen_in, YY_BUF_SIZE));
              }
            }
          }
        }
      }

      globfree(&glob_buf);
    } else {
      filter_error(pos, "stat failed on \"%s\": %s", name, strerror(errno));
    }
  } else {
    if (S_ISDIR(st.st_mode)) {
      char *b = strdup(name);
      char *base = basename(b);

      if (strcmp("/", base) == 0) {
        filter_error(pos, "cannot include / path; skipping");
        free(b);
        return;
      }
      free(b);

      if ((n = scandir(name, &namelist, NULL, alphasort)) < 0) {
        filter_error(pos, "scandir failed on \"%s\": %s", name,
                     strerror(errno));
      } else {
        while (n--) {
          /* FIXME: assumes d_name */
          if (namelist[n]->d_name[0] == '.') {
            free(namelist[n]);
            continue;
          }
          if (asprintf(&fn, "%s/%s", name, namelist[n]->d_name) < 0) {
            filter_error(
                NULL, "internal error: asprintf failed constructing pathname "
                      "for included file \"%s\"",
                namelist[n]->d_name);
            free(fn);
          } else {
            include_file(yylloc, fn);
            free(fn);
          }
          free(namelist[n]);
        }
        free(namelist);
      }
    } else {
      if (sourcefile_push(pos, name)) {
        filtergen_in = current_srcfile->f;
        filtergen_lineno = current_srcfile->lineno;
        //        yycolumn = current_srcfile->column;
        if (!filtergen_in) {
          filter_error(pos, "failed to open %s", name);
        } else {
          filtergen_push_buffer_state(
              filtergen__create_buffer(filtergen_in, YY_BUF_SIZE));
        }
      }
    }
  }
}
