/* filtergen
 *
 * Copyright (c) 2003-2010 Jamie Wilkinson <jaq@spacepants.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

int
main()
{
  // Get the top level suite from the registry
  CppUnit::Test * suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  // add the test to the list of test to run
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  // Change the default outputter to a compiler error format outputter
  CppUnit::CompilerOutputter * outputter = new CppUnit::CompilerOutputter(&runner.result(), std::cerr);
  // cppunit decided that shortening the gcc output to just the filename
  // was a good idea.  very unuseful.  %p is the full path of the source file.
  outputter->setLocationFormat("%p:%l:");
  runner.setOutputter(outputter);


  // Run the tests
  bool wasSuccessful = runner.run();

  // return error code 1 if one of the tests failed
  return wasSuccessful ? 0 : 1;
}

