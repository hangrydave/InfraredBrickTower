# Infrared Brick Tower

Infrared Brick Tower is a project to bring new life to the Lego Mindstorms RCX 2.0 USB infrared transmission tower for modern versions of Windows.

(this thing)
![An image of the Lego USB infrared transmission tower](https://github.com/hangrydave/InfraredBrickTower_WinUSB/blob/main/tower.jpg?raw=true)

I'm building this project because, first of all, it's fun, and Legos (I know Legos isn't the correct plural term, but it's fun to say; fight me in the GitHub issues) are pretty cool. the other big reason that I think there's a need for this is that there's barely any community support for this Lego Mindstorms generation nowadays (let alone official support), and there's no way to use the USB version of the tower on 64-bit Windows. (Allegedly the serial version of the tower works on modern Windows with an adapter, but I don't have one to test.)

This project is currently built off of WinUSB, but at some point I'd like to shift over to UMDF 2 or something. (that's not gonna happen until Microsoft fixes the regression at the top of this page though, I can't be bothered to downgrade Visual Studio: https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)

I'd like this to be useful to other people than myself, but this is first and foremost a passion project. Current goals are to support downloading compiled RCX programs and bytecode, and to communicate in VLL with the Micro Scout. Also, (for the better or worse of my own sanity) I'm planning on writing a [NQC](http://bricxcc.sourceforge.net/nqc/) compiler at some point.


## Contributing
If you have changes you'd like to make or recommendations, feel free to open a pull request or an issue!

## License
[GNU GPL v3.0](https://choosealicense.com/licenses/gpl-3.0/)
