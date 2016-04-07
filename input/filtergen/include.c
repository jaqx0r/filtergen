#include "include.h"

/* include a file or directory */
void include_file(const char *name) {
  struct stat st;
  struct dirent **namelist;
  char *fn;
  int n;

  if (stat(name, &st)) {
    if (errno == ENOENT &&
        (index(name, '*') != NULL || index(name, '?') != NULL ||
         index(name, '[') != NULL)) {
      /* Globbing fiesta! */
      glob_t glob_buf;
      if (glob(name, 0, NULL, &glob_buf) != 0) {
        filtergen_error("failed to glob \"%s\": %s", name, strerror(errno));
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
            filtergen_warn("stat failed on globbed \"%s\": %s",
                           glob_buf.gl_pathv[n], strerror(errno));
          } else if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) {
            if (sourcefile_push(glob_buf.gl_pathv[n])) {
              yyin = current_srcfile->f;
              yylineno = current_srcfile->lineno;
              yycolumn = current_srcfile->column;
              if (!yyin) {
                filtergen_error(NULL, "failed to open %s",
                                glob_buf.gl_pathv[n]);
              } else {
                yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
              }
            }
          }
        }
      }

      globfree(&glob_buf);
    } else {
      filtergen_warn("stat failed on \"%s\": %s", name, strerror(errno));
    }
  } else {
    if (S_ISDIR(st.st_mode)) {
      char *b = strdup(name);
      char *base = basename(b);

      if (strcmp("/", base) == 0) {
        filtergen_warn("cannot include / path; skipping");
        free(b);
        return;
      }
      free(b);

      if ((n = scandir(name, &namelist, NULL, alphasort)) < 0) {
        filtergen_warn("scandir failed on \"%s\": %s", name, strerror(errno));
      } else {
        while (n--) {
          /* FIXME: assumes d_name */
          if (namelist[n]->d_name[0] == '.') {
            free(namelist[n]);
            continue;
          }
          if (asprintf(&fn, "%s/%s", name, namelist[n]->d_name) < 0) {
            filtergen_error(
                NULL, "internal error: asprintf failed constructing pathname "
                      "for included file \"%s\"",
                namelist[n]->d_name);
            free(fn);
          } else {
            include_file(fn);
            free(fn);
          }
          free(namelist[n]);
        }
        free(namelist);
      }
    } else {
      if (sourcefile_push(name)) {
        yyin = current_srcfile->f;
        yylineno = current_srcfile->lineno;
        yycolumn = current_srcfile->column;
        if (!yyin) {
          filtergen_error(NULL, "failed to open %s", name);
        } else {
          yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
        }
      }
    }
  }
}
