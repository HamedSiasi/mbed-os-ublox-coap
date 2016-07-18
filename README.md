# mbed-os: u-blox Test Application

This repo contains an application that pulls together all the bits of the u-blox integration of the new mbed 5 release and builds something runnable.

# Prerequisites
To fetch and build the code in this repository you need first to install the [mbed CLI tools](https://github.com/ARMmbed/mbed-cli#installation) and their prerequisites.

# Repo Structure
The repo structure is quite confusing; it goes like this:

* What you see here is the top level. It contains only the test application.
* Look for the vital little file `mbed-os.lib`.  It really is vital.
* Because github can't do references to repos from within repos (like good 'ole SVN did), ARM have invented these `.lib` files.  The name is very confusing as all the file contains is a link to another repo (`lnk` would have been a better file extension to choose).  When you use the [mbed command line tools](https://github.com/ARMmbed/mbed-cli) or the mbed on-line IDE it understands these `.lib` files.  So the effect of `mbed-os.lib` is to tell the mbed tools to go get the github URL inside the `mbed-os.lib` file and put it in the sub-directory `mbed-os`.  In the on-line IDE this is called a library, hence the confusing name.
* So, you MUST use the [mbed command line tools](https://github.com/ARMmbed/mbed-cli) to sort everything out for you.  For instance, to create a new mbed-os application for yourself, let's call it `my-app`, create the `my-app` directory, `cd` to it and then (assuming you have the mbed CLI tools installed) enter `mbed new .`.  This will go and get all of the latest mbed-os release and put it into the correct sub-directories.  All you need to do then is add your application file(s) to the top-level of directory and you have a code tree which should compile and run 'on' mbed-os.
* However, we're in mid port and our stuff hasn't yet made it into the main mbed-os release, so you need to do a little shimmy before you can begin.  All of the mbed _platform_ stuff, which is now aligned with the mbed 2 codebase, is in a sub-directory of its own off the `mbed-os` directory called `core`.  As above, the URL of the repo containing `core` (which is actually the latest [mbed 2 release](https://github.com/mbedmicro/mbed)) is referred to by a file `core.lib` and the mbed tools sort it all out for you.  So, rather than getting the current release of `core` you need instead to get the branch of it with all our stuff in.  To do this, `cd` to the `mbed-os` sub-directory.  In there enter `mbed update workshop-ublox`.  This will switch the code base away from the `mbed-os` master branch to our u-box branch where we have integrated u-blox-ness.  Actually, the sole change on that branch is to modify the file `core.lib` to point to [our _fork_ of the mbed 2 platform code](https://github.com/ARMmbed/mbed-ublox), which is where our integration has been done.  Now `cd` to the `core` sub-directory and in there enter `git checkout master`.  This will switch you to using the master branch of our u-blox fork.  Finally, enter `mbed update` once more to actually update the code on your disk to the u-blox master.

# Building This Code
Once you have cloned this repo, `cd` to the repo directory and enter the following:

`mbed update`

This will fetch the latest mbed-os release (from the URL in the `mbed-os.lib` file, recursing as necessary).

Then, switch to the u-blox integration version of the mbed-os by entering the following:

`cd mbed-os`

`mbed update workshop-ublox`

`cd core`

`git checkout master`

`mbed update`

Having done that, `cd` your way back to the top-level repo directory once more.

You need to set the target and the toolchain that you want to use.  The target and toolchain we'd like to use this application on is `SARA_NBIOT_EVK` and the toolchain is `GCC_ARM`.  Of course, in future we will support the u-blox targets on other toolchains (e.g. ARM, microVision, IAR) and, equally, if there's another target you happen to be using, exactly the same application code can be used on that target.  To get a list of supported targets and their toolchains enter `mbed compile -S`.

You can set the target and toolchain for this application once by entering the following two commands (while in the top-level directory of the repo):

`mbed target SARA_NBIOT_EVK`

`mbed toolchain GCC_ARM`

Once this is done, build the code with:

`mbed compile`

You will find the output files in the sub-directory `.build\SARA_NBIOT_EVK\GCC_ARM\`.

# Other Things

* As a result of the little shimmy above to use our integration fork of the mbed-os release, you will have a modified `core.lib` file in your `mbed-os` sub-directory.  Don't check that in (you probably can't anyway).
* There is a file `mbed-settings.py` in the top-level repo directory.  This is where you make local changes to how you want the code built.  The only non-default setting in this particular file is to change:

  `BUILD_OPTIONS = []`

  to

  `BUILD_OPTIONS = ["debug-info"]`

  ...which will get debug output into the `.elf` file and switch optimisation off so that you can use a debugger.
* Eclipse project files are included but you can also build from the command-line as above.
* There is currently an issue where attempts to `malloc()` more than a total of 10324 bytes will succeed (up to 11572 bytes) by overlapping into the stack area.  While it is correct to overlap into the stack area, it shouldn't go that far.  This issue has been raised with ARM in [https://github.com/ARMmbed/mbed-os/issues/464](https://github.com/ARMmbed/mbed-os/issues/464).