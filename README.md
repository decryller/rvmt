## RVMT
Stands for "Revamped Terminal" (for X11).\
I don't think it offers that much to the terminal to call it a revamp, but I like how "RVMT" looks so whatever.
### Dependencies
- libx11

### Including in your project
1) Include `rvmt.cpp` and `rvmt.hpp` in your project folder.
2) Include `rvmt.hpp` in your main file. 
2) Compile `rvmt.cpp` along with your main file.
```
$ g++ rvmt.cpp -lX11 <your arguments...>
```
### Running the example
Try out the example (`example.cpp`).
```
$ g++ rvmt.cpp example.cpp -lX11 -o rvmtTest
$ ./rvmtTest
```
### Fully functional on
#### Window managers:
- i3-wm
- xfce4
#### Terminals:
- Alacritty
- Kitty
- xfce4-terminal
- gnome-terminal
- Terminator

## Note
I wanted a clickable terminal interface for another project and I ended up doing all of this, and I thought why not give it its own repo. Once I'm steady with the other project I will definitely come around here and give some more dedication to this project & readme\
The other project: [Wraith](https://github.com/decryller/wraith)
## Contact
E-mail: decryller@gmail.com\
Discord: decryller
