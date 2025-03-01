/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */



#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "command.hh"
#include "shell.hh"


Command::Command() {
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;

    _background = false;
	_append = 0;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    _simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    _simpleCommands.clear();

    if ( _outFile ) {
        delete _outFile;
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
	_append = 0;
}



void Command::print() {
/*
    	printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
*/
    }


bool Command::builtIn(int i) {
	//TODO

	if( strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "setenv") == 0 ) {
		if (setenv(_simpleCommands[i]->_arguments[1]->c_str(), _simpleCommands[i]->_arguments[2]->c_str(), 1)) {
			perror("setenv");
		}
		clear();
		Shell::prompt();
		return true;
	}

	if( strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "unsetenv") == 0 ) {
		if (unsetenv(_simpleCommands[i]->_arguments[1]->c_str())) {
			perror("unsetenv");
		}
		clear();
		Shell::prompt();
		return true;
	}

	if (strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "cd") == 0) {

		int error;
		if (_simpleCommands[i]->_arguments.size() == 1) {	//if only "cd", then go HOME
			error = chdir(getenv("HOME"));
		}
		else {
			error = chdir(_simpleCommands[i]->_arguments[1]->c_str());
		}

		if (error < 0) {
			perror("cd");
		}

		clear();
		return true;
	}

	return false;
}

bool Command::builtIn2(int i) {
	if (strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "printenv") == 0) {
		char ** envvar = environ;

		int i = 0;
		while (envvar[i] != NULL) {
			printf("%s\n", envvar[i]);
			i++;
		}
		return true;
	}
	return false;
}

void Command::execute() {

	if (_simpleCommands.size() == 0) {
		Shell::prompt();
		return;
	
	}

	int dfltin = dup(0);
	int dfltout = dup(1);
	int dflterr = dup(2);

	int fdin = 0;
	int	fdout = 0;
	int fderr = 0;

	if (_inFile) {
		fdin = open(_inFile->c_str(), O_RDONLY);
	}
	else {
		fdin = dup(dfltin);
	}

	if (_errFile) {
		if (_append) {
			fderr = open(_errFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0600);
		}
		else {
			fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
		}
	}
	else {
		fderr = dup(dflterr);
	}

	dup2(fderr, 2);
	close(fderr);

	int pid;

	for (size_t i = 0; i < _simpleCommands.size(); i++) {

		dup2(fdin, 0);
		close(fdin);

		if (i == _simpleCommands.size() - 1) {
			if (_outFile) {
				if (_append) {
					fdout = open(_outFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0600);
				}
				else {
					fdout = open(_outFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
				}
			}
			else {
				fdout = dup(dfltout);
			}
		}
		else {
			int fdpipe[2];
			pipe(fdpipe);
			fdout = fdpipe[1];
			fdin = fdpipe[0];
		}	

		dup2(fdout, 1);
		close(fdout);

		if (builtIn(i)) {
			return;
		}

		pid = fork();

		if (pid == -1) {
			perror("fork\n");
			exit(2);
		}

		if (pid == 0) {

			if (builtIn2(i)) {
				return;
			}

			if (strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "source") == 0) {
				FILE * fp = fopen(_simpleCommands[i]->_arguments[1]->c_str(), "r");

				char cmdline[1024];
				fgets(cmdline, 1023, fp);
				fclose(fp);

				int tempin = dup(0);
				int tempout = dup(1);

				//pipe
				int fdpipein[2];
				pipe(fdpipein);
				int fdpipeout[2];
				pipe(fdpipeout);

				write(fdpipein[1], cmdline, strlen(cmdline));
				write(fdpipein[1], "\n", 1);
				close(fdpipein[1]);

				dup2(fdpipein[0], 0);
				close(fdpipein[0]);
				dup2(fdpipeout[1], 1);
				close(fdpipeout[1]);

				int ret = fork();
				if (ret == 0) {
					execvp("/proc/self/exe", NULL);
					_exit(1);
				}
				else if (ret < 0) {
					perror("fork");
					exit(1);
				}

				dup2(tempin, 0);
				dup2(tempout, 1);
				close(tempin);
				close(tempout);

				char ch;
				char * buffer = (char *)malloc(4096);
				int i = 0;

				while (read(fdpipeout[0], &ch, 1)) {
					if (ch != '\n') {
						buffer[i++] = ch;
					}
				}

				buffer[i] = '\0';
				printf("%s\n", buffer);

			}
			else {
					size_t argsize = _simpleCommands[i]->_arguments.size();
					char ** x = new char*[argsize + 1];
					for (size_t j = 0; j < argsize; j++) {
						x[j] = const_cast<char*>(_simpleCommands[i]->_arguments[j]->c_str());
						x[j][strlen(_simpleCommands[i]->_arguments[j]->c_str())] = '\0';

					}
					x[argsize] = NULL;
					execvp(x[0], x);
					_exit(1);	

			}	
		}	
	}	

	dup2(dfltin, 0);
	dup2(dfltout, 1);
	dup2(dflterr, 2);
	close(dfltin);
	close(dfltout);
	close(dflterr);

	if (_background == false) {
		waitpid(pid, NULL, 0);
	}
	clear();

	Shell::prompt();
}

SimpleCommand * Command::_currentSimpleCommand;
