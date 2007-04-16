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

