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
const std::string CREATE_DIR_CMD = "mkdir";
const std::string REMOVE_DIR_CMD = "rmdir";
const std::string MOVE_CMD = "mv";

const std::string HELP_STRING = "The following commands are supported: \n"
	+ LIST_CMD + " [<directory>] - list directory content. \n"
	+ CONTENT_CMD + " <path> - show file content. \n"
	+ CREATE_FILE_CMD + " <path> - create empty file. \n"
	+ EDIT_CMD + " <path> - re-set file content. \n"
    + REMOVE_CMD + " <path> - remove file. \n"
    + CREATE_DIR_CMD + " <path> - create new directory. \n"
    + REMOVE_DIR_CMD + " <path> - remove directory. \n"
    + MOVE_CMD + " <old_path> <new_path> - move file. \n"
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
                if (cmd.size() == 1) {
                    fs.list_dir("/");
                } else if (cmd.size() == 2) {
                    fs.list_dir(cmd[1]);
                } else {
                    std::cout << "Invalid command format. Usage: ls [<directory>]" << std::endl;
                }
            } else if (cmd[0] == CREATE_FILE_CMD) {
                if (cmd.size() == 2) {
                    fs.create_file(cmd[1], false);
                } else {
                    std::cout << "Invalid command format. Usage: touch <path> or touch <path> dir" << std::endl;
                }
			} else if (cmd[0] == CONTENT_CMD) {
                if (cmd.size() == 2) {
                    try {
                        std::string content = fs.get_content(cmd[1]);
                        std::cout << content << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "Error: " << e.what() << std::endl;
                    }
                } else {
                    std::cout << "Invalid command format. Usage: cat <path>" << std::endl;
                }
			} else if (cmd[0] == EDIT_CMD) {
                if (cmd.size() == 2) {
                    std::string new_content;
                    std::cout << "Enter new content (type EOF to finish):\n";
                    while (true) {
                        std::string line;
                        std::getline(std::cin, line);
                        if (line == "EOF") {
                            break;
                        }
                        new_content += line + "\n";
                    }
                    try {
                        fs.set_content(cmd[1], new_content);
                        std::cout << "Content updated successfully.\n";
                    } catch (const std::exception& e) {
                        std::cout << "Error: " << e.what() << std::endl;
                    }
                } else {
                    std::cout << "Invalid command format. Usage: edit <path>" << std::endl;
                }
			} else if (cmd[0] == REMOVE_CMD) {
                if (cmd.size() == 2) {
                    fs.remove_file(cmd[1]);
                } else {
                    std::cout << "Invalid command format. Usage: rm <path>" << std::endl;
                }
            } else if (cmd[0] == CREATE_DIR_CMD) {
                if (cmd.size() == 2) {
                    try {
                        fs.create_file(cmd[1], true);
                    } catch (const std::exception& e) {
                        std::cout << "Error creating directory: " << e.what() << std::endl;
                    }
                } else {
                    std::cout << "Invalid command format. Usage: mkdir <directory_path>" << std::endl;
                }
            } else if (cmd[0] == REMOVE_DIR_CMD) {
                if (cmd.size() == 2) {
                    try {
                        fs.remove_directory(cmd[1]);
                        std::cout << "Directory removed: " << cmd[1] << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "Error removing directory: " << e.what() << std::endl;
                    }
                } else {
                    std::cout << "Invalid command format. Usage: rmdir <directory_path>" << std::endl;
                }
            } else if (cmd[0] == MOVE_CMD) {
                if (cmd.size() == 3) {
                    try {
                        fs.move_file(cmd[1], cmd[2]);
                        std::cout << "Moved: " << cmd[1] << " to " << cmd[2] << std::endl;
                    } catch (const std::exception &e) {
                        std::cout << "Error moving: " << e.what() << std::endl;
                    }
            }else {
                    std::cout << "Invalid command format. Usage: mv <source_path> <destination_path>" << std::endl;
                }
            }else {
				std::cout << "unknown command: " << cmd[0] << std::endl;
			}
		} catch (std::runtime_error &e) {
			std::cout << e.what() << std::endl;
		}
	}
}
