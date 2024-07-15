#include "vfs.h"

#include <iostream>
#include <sstream>


const std::string FS_NAME = "myfs";

const std::string LIST_CMD = "ls";
const std::string CONTENT_CMD = "cat";
const std::string CREATE_FILE_CMD = "touch";
const std::string EDIT_CMD = "edit";
const std::string REMOVE_CMD = "rm";
const std::string HELP_CMD = "help";
const std::string EXIT_CMD = "exit";

const std::string HELP_STRING = "The following commands are supported: \n"
	+ LIST_CMD + " [<directory>] - list directory content. \n"
	+ CONTENT_CMD + " <path> - show file content. \n"
	+ CREATE_FILE_CMD + " <path> - create empty file. \n"
	+ EDIT_CMD + " <path> - re-set file content. \n"
    + REMOVE_CMD + " <path> - remove file. \n"
	+ HELP_CMD + " - show this help messege. \n"
	+ EXIT_CMD + " - gracefully exit. \n";


std::vector<std::string> split_cmd(std::string cmd) {
	std::stringstream ss(cmd);
	std::string part;
	std::vector<std::string> ans;

	while (std::getline(ss, part, ' '))
		ans.push_back(part);

	return ans;
}

void run_vfs(MyFs &fs) {
	std::cout << "Welcome to " << FS_NAME << std::endl;
	std::cout << "To get help, please type 'help' on the prompt below." << std::endl;
	std::cout << std::endl;

    bool exit = false;
	while (!exit) {
		try {
			std::string cmdline;
			std::cout << FS_NAME << "$ ";
			std::getline(std::cin, cmdline, '\n');
			if (cmdline == std::string(""))
				continue;

			std::vector<std::string> cmd = split_cmd(cmdline);

            // Add the relevant calls to MyFs object in these ifs

			if (cmd[0] == EXIT_CMD) {
				exit = true;
			} else if (cmd[0] == HELP_CMD) {
				std::cout << HELP_STRING;
			} else if (cmd[0] == LIST_CMD) {
                // Add your code here - call a MyFs function and don't put the implementation here
				throw std::runtime_error("not implemented");
            } else if (cmd[0] == CREATE_FILE_CMD) {
                // Add your code here - call a MyFs function and don't put the implementation here
				throw std::runtime_error("not implemented");
			} else if (cmd[0] == CONTENT_CMD) {
                // Add your code here - call a MyFs function and don't put the implementation here
				throw std::runtime_error("not implemented");
			} else if (cmd[0] == EDIT_CMD) {
                // Add your code here - call a MyFs function and don't put the implementation here
				throw std::runtime_error("not implemented");
			} else if (cmd[0] == REMOVE_CMD) {
                // Add your code here - call a MyFs function and don't put the implementation here
				throw std::runtime_error("not implemented");
            } else {
				std::cout << "unknown command: " << cmd[0] << std::endl;
			}
		} catch (std::runtime_error &e) {
			std::cout << e.what() << std::endl;
		}
	}
}
