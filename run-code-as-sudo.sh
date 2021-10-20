#/bin/sh
echo "Attention, this will run vscode as root, so all extensions must be reinstalled"
echo "In order to run this project the essentials extensions are:"
echo "  *    C/C++ (from: Microsoft)"
echo "  *    C/Easy C++ projects (from: Alejandro Charte Luque)"
sudo code . --user-data-dir='/home/root/.vscode-root/' --no-sandbox
