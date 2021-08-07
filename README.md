# Shell-Implementation

This repository documents a basic Shell implementation in C as part of Operation Systems course I took at Bar-Ilan University.

The program shows a prompt marker ("$" sign) that means it is waiting for the user to enter Linux commands.
The commands are seperated to two types:
1. <b>Built-In commands</b> - commands that are "hard-coded" in the program and executed by the parent process.
2. <b>Non-Built-In commands</b> - commands that are implemented elsewhere and designed to run in a child process.

The user can run a Non-Built-In command in two ways:
1. <b>Foreground</b> - the command will be executed by a child process and the parent will wait until the childs is done, means the user cannot enter new commands until the current operation is done. 
2. <b>Background</b> - the command will be executed by a child process and the parent will not wait for the child to finish, means the operation is running in the background and the user is allowed to enter another command before the operation is done. To ask the shell to run the command X in the background, the user has to enter "X &".

Built-In Commands:
1. jobs - shows the commands that have been entered and still running in the background.
2. history - shows all the commands that have been entered since the program started, with a "RUNNING/DONE" marker.
3. cd - changes the program's current working directory.
4. exit - stops the program.


## Instructions

1. Download "MyShell.c" file from this repo.
2. Compile it with the command "gcc Myfile.c" (Linux).
3. Run it with the command "a.out".


## Screenshot

![image](https://user-images.githubusercontent.com/72878018/128595671-a80517e8-5528-4b4e-a6ae-60a14a8bc677.png)


## IDE, Writers and Tools

1. Visual Studio Code
2. Notepad++
3. WSL2
