CE USART Project                        {#mainpage}
================

This project is based on Contextual Eelectornics course that brings up a the USART peripheral on STM32 micro.

In this specific example the board I have is a Nucleo F446ZE and this particular project implements a dead simple "serial interface".

The board is setup to run at a 115200 buadrate and is essentially an echo server with the following characters additionally toggling leds

* 1 -> Toggles Green Led on board
* 2 -> Toggles Blue Led on board
* 3 -> Toggles Red Led on board

On boot the board will will setup the Initialize all the device and then print a header out to the USART 3 device that includes the firmware and hardware version along with the date compiled.

It's basic but a start.

[Repo this is based off of](https://github.com/ContextualElectronics/Embedded/tree/master/USART/Lesson_2)