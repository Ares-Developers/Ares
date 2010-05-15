A R E S  R E A D M E
--------------------

For credits and further information on how to use Ares with your mod,
read AresManual.html.

______________________________________________________________________
ATTENTION: If you're reading this, you very most likely downloaded the 
standalone version of Ares. It is VERY HIGHLY RECOMMENDED that you use 
Launch Base to use Ares instead.
You can get Launch Base from here: 
   http://marshall.strategy-x.com/LaunchBase/
______________________________________________________________________


USING THE STANDALONE VERSION OF ARES
------------------------------------

Since you are reading this readme, it is assumed you have already
downloaded the Ares Standalone Package.

1. Extract the Ares Standalone Package into your YR game directory.
   Make sure Syringe.exe, Ares.dll, Ares.dll.inj and ares.mix are all directly
   in that directory, not in a sub-folder.

2. Ensure that you have installed the following runtime files from Microsoft:
   http://www.microsoft.com/downloads/details.aspx?FamilyID=9b2da534-3e03-4391-8a4d-074b9f2bc1bf&displaylang=en

3. Ensure that gamemd.exe and syringe.exe are both set to the same Compatibility Mode.

4. Click in the Windows Explorer's address bar while you're in the game 
   directory. Type 
       Syringe "gamemd.exe"
   and press Enter. 
   Or make a bat file to do that for you... no difference.


RUNNING ARES THROUGH LAUNCH BASE
--------------------------------

-
NOTE: Launch Base will not run Syringe (or Ares) by default - it only runs 
Syringe if the mod you are launching requires Ares.
-

1. Download and install the latest version of Launch Base.
   http://marshall.strategy-x.com/LaunchBase/LB_Setup.exe

2. Run Launch Base and then launch unmodded Yuri's Revenge to make sure you
   don't have any trouble with basic Launch Base functions. 
   Configure Launch Base as you see fit (Tools > Options).

3. In the main window, click Ares > Update Syringe & Ares 
   to make sure the Ares download has been sucessful.

If a mod requires Ares, Launch Base should now properly launch it.

CREATING A MOD WITH ARES
------------------------

-
The information below pertains specifically to creating a Launch Base-compatible
mod with Ares. If you do not wish to do that, you can now switch to 
AresManual.html and learn all about Ares's changes and additions.
-

Everything below should usually not be done manually, and instead be left to 
Launch Base Mod Creator (which you can get via Tools > Check For Updates).


1. Go to your Launch Base program folder and then "Mods"
   Inside the "Mods" folder, create a new directory for your mod.
   Inside that folder, create two folders: "launcher" and "video".

2. In the newly created "launcher" folder, create a flat text file named 
   "liblist.gam". Inside "liblist.gam", copy and paste the following text:

[General]
Name=YOUR MOD NAME
Description=YOUR MOD DESCRIPTION
ModType=mod
AllowTX=yes
UseAres=yes

   These are just the most basic options. There are many other flags that can 
   be used to customise your mod - experiment with Launch Base Mod Creator to 
   see what options you have.

3. Write some modifications. As it is, your mod can now be used to launch the 
   unmodded game with the latest release of Ares. Many of the new features of 
   Ares, however, require some changes to the INIs and/or the inclusion of 
   additional files. (See AresManual.html for more information.)

4. For now, you should place all mod files inside the "video" folder. In future,
   DCoder's DLL will be used to automatically generate MIX files at which point
   you will need to place mod files in different folders. Anything placed in the
   "video" folder will be copied loose to the Red Alert 2 folder.
   
   In other words: Your mod should reside in the "video" folder in a state in
   which the game could read it, were it in the game folder.
   (All files properly mix'd, etc.)

5. Test away!
   If you have any problems or questions about the above instructions or using 
   Ares with Launch Base then please post them in the forums. Problems/questions 
   with Launch Base in general please post in the Launch Base beta test thread.


--------------------------------------------------------------------------------

Please mind the licenses of all involved files, RTFM before asking questions,
file bugs in the Bugtracker (not the forums), and post clearly and coherently.

We will happily answer questions about Ares at the forums or in chat, but
THERE IS NO OFFICIAL NPATCH > ARES MIGRATION SUPPORT.
The reasons for this have been discussed at length and will not be reiterated
in this readme. If you wish to migrate from NPatch, feel free to coordinate
your efforts with other NPatch users.


Most importantly, enjoy Ares! :)
The Ares Development Team


URLS
----
Ares: http://ares.strategy-x.com/
Bugtracker: http://bugs.renegadeprojects.com/
Forums: http://forums.renegadeprojects.com/
ModEnc: http://www.modenc.renegadeprojects.com/
Chat: http://bit.ly/bqsDua (Browser) or irc://irc.burnirc.net/RenProj (IRC Client)
Launch Base: http://marshall.strategy-x.com/LaunchBase/

--------------------------------------------------------------------------------
For information about copyrights and other legal information, read the respective
license files.

EOF