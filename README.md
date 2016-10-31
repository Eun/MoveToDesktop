MoveToDesktop [![PayPal donate button](https://img.shields.io/badge/donate-paypal-blue.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=eun%40su%2eam&lc=US&item_name=MoveToDesktop%20Donation&no_note=0&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donate_SM%2egif%3aNonHostedGuest "Donate with PayPal") [![Gittip donate button](https://img.shields.io/gratipay/Eun.svg)](https://gratipay.com/Eun/ "Donate weekly to this project using Gittip") [![Flattr](https://img.shields.io/badge/donate-flattr-green.svg)](https://flattr.com/submit/auto?user_id=Eun&url=https%3A%2F%2Fgithub.com%2FEun%2FMoveToDesktop "Flattr this") [![Join the chat at https://gitter.im/Eun/MoveToDesktop](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/Eun/MoveToDesktop?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
==============
Adds the Move to Desktop feature to the Windows 10 System menu:


![](https://raw.githubusercontent.com/Eun/MoveToDesktop/res/screenshot1.png)

[![Download](https://raw.githubusercontent.com/Eun/MoveToDesktop/res/download.png)](https://github.com/Eun/MoveToDesktop/releases/download/1.4/MoveToDesktop-1.4.zip)

Installation Usage
======
Download and Run.  
You can also move windows by using <kbd>WIN</kbd>+<kbd>ALT</kbd>+<kbd>Left/Right</kbd>  


> **Hint**  
> It is usefull to start MoveToDesktop as Administrator.  
> If you do so, MoveToDesktop is also enabled for privileged tasks.  
> For starting MoveToDesktop on logon the prefered way is to create a scheduled task. [See how](help/scheduled-tasks.md). 

Settings
========
You can place the [MoveToDesktop.ini](MoveToDesktop.ini) into `%AppData%` and modify the settings in it.  
A restart of the application is required.

Changelog
=========
1.4:
* Update for Windows 10 Anniversary (14393)

1.3:
* Delete empty desktops
* Bufix Issue #8
* No Redistributable needed anymore #12
* Setting for new Desktops #10

1.2:
* Configurable Hotkeys
* Mutex bugfix

1.1:
* Hotkey
* Keyboard accelators
* Switch desktop after move
* Settings Ini File

1.0:
* Release
