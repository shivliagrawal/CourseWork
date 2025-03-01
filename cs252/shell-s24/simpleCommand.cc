

#include <cstdio>
#include <cstdlib>

#include <iostream>

#include "simpleCommand.hh"

SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}


void SimpleCommand::insertArgument( std::string * argument ) {
/*
	char *arg = const_cast<char*> (argument->c_str());
	printf("the argument to be inserted is: %s\n", arg);

	3.1: environment variable expansion
	std::string * envexp = envexpansion(argument);

	if (envtrue == true) {
		argument = envexp;
	}

	3.2: tilde expansion
*/

	_arguments.push_back(argument);
}

void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << arg << "\" \t";
  }
  std::cout << std::endl;
}



/*


SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  for (auto & arg : _arguments) {
    delete arg;
  }
}

void SimpleCommand::insertArgument( std::string * argument ) {
 	simply add the argument to the vector

	char *arg = const_cast<char*> (argument->c_str());
	printf("the argument to be inserted is: %s\n", arg);

	3.1: environment variable expansion
	std::string * envexp = envexpansion(argument);

	if (envtrue == true) {
		argument = envexp;
	}

	3.2: tilde expansion


	envtrue = false;
	tildtrue = false;
  _arguments.push_back(argument);
}

void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << arg << "\" \t";
  }
  std::cout << std::endl;
}

*/
