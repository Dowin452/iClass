sudo pacman -Syu
sudo pacman -S git gcc
sudo pacman -S python python-pip
pip install windows-curses
git clone https://github.com/Dowin452/iClass
cd iClass
chmod +x iclass.py
echo "Type ./iclass to run the program"
