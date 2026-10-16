Steps

1. Create my_user_kernel.c.
2. Add the kernel module code and save the file.
3. Create the Makefile and add the build instructions.
4. Compile the kernel module using make.
5. Verify the generated module using ls -l my_user_kernel.ko.
6. Load the module using sudo insmod my_user_kernel.ko.
7. Verify the proc entry using ls -l /proc/my_user_kernel.
8. Read the initial message using cat /proc/my_user_kernel.
9. Write a message using echo "Hello from user space!" | sudo tee /proc/my_user_kernel.
10. Read the stored message using cat /proc/my_user_kernel.
11. Verify kernel logs using sudo dmesg | tail -10.
12. Unload the module using sudo rmmod my_user_kernel.
13. Verify that /proc/my_user_kernel has been removed.
14. Clean generated build files using make clean.
