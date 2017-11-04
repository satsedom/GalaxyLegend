Galaxy Legend is built for Android OS and is coded at multiple layers, each level utilizing a different language, but all serve a specific purpose.

Under the hood it uses native code libraries built in C. These handle compression, encryption, OS / Java / LUA / Flash interactions, etc.

Java is used to interact with OS, Google, etc.

Game logic is coded in LUA and Flash.

LUA compiled modules are stored as TFL files with contents Cyphered/Encrypted and in some cases Compressed.

Flash compiled modules are stored as TFS files with contents Compressed.

Text strings are split into index and language specific files. 

Current version is 1.9.0.

In this release, all LUA modules and parsed Text strings are available as examples of decompiled code.

Note: some of the LUA modules are loaded at runtime, they are not currently included in this example.

________________________________________________________________________________

Following are some steps in getting this work done yourself. I will add more steps as I get time to describe them.

Simple things first.

Download and extract apk:
```
Create temp directory, say c:\temp\gl\gl190\.
Download latest apk from https://www.apk4fun.com to c:\temp\gl\gl190\gl190.apk.
Download and install 7zip, WinZip, or WinRar.
Extract contents of apk file into c:\temp\gl\gl190\.
Extract contents of c:\temp\gl\gl190\assets\tap4fun.zip into c:\temp\gl\gl190\assets\.
This will create a file structure similar as the path on an Android device: tap4fun\galaxylegend\AppOriginalData\.
```

Compile texttool program:
```
Download and install Java JDK 7 or higher.
javac texttool.java
```

Parse Text index and language files into readable CSV format using texttool:
```
Execute for each text file:
java texttool c:\temp\gl\gl190\assets\tap4fun\galaxylegend\AppOriginalData\data2\text\item.idx c:\temp\gl\gl190\assets\tap4fun\galaxylegend\AppOriginalData\data2\text\item.en > item.csv
```

Difficult things next.

Notes:
```
PAK files are archives of files using Gameloft/Glitch Engine Rotating XOR Encryption/Decryption.
TFL files are LUA binaries using Gameloft/Glitch Engine Rotating XOR Encryption/Decryption.
```

Install Unix Shell on Windows:
```
Download and install Cygwin. This makes it easy to create and execute scripts.
Choose to install gcc. We need 32bit C compiler specifically.
Choose to install make.
...More details to come about installing...
```

Compile xortool program:
```
In the meantime, you can use the attached binary.
...More details to come about compiling...
```

Extract PAK archives using xortool:
```
cd c:\temp\gl\gl190\assets\tap4fun\galaxylegend\AppOriginalData\
xortool data.pak .\data\ 4
xortool data1.pak .\data1\ 4
```

Decrypt TFL files into LUAQ binary executable format using xortool:
```
Execute for each TFL file:
cd c:\temp\gl\gl190\assets\tap4fun\galaxylegend\AppOriginalData\data1\
xortool AutoUpdateInit.tfl AutoUpdateInit.luaq 2
```

Modify LUA decompiler Opcodes from defaults to T4F rearranged codes:
```
LUA source code is available online. T4F is using version 5.1.
Part of the source code is Opcodes configuration file.
A C structure defines opcodes in some default order.
T4F had rearranged opcodes from the default order, to prevent an easy opportunity decompile.
This order can be determined from LUA imports in the C library.
Similar, the decompiler needs to match this configuration.
Decompiler I am using can be downloaded here: https://sourceforge.net/projects/unluac/.
I had retrofitted its opcodes to match the rearrangement.
```

Compile decompiler program:
```
Download unluac folder from repository.
javac unluac\Main.java
```

Decompile LUAQ executables into readable LUA programs:
```
Execute for each LUAQ file:
cd c:\temp\gl\gl190\assets\tap4fun\galaxylegend\AppOriginalData\data1\
java unluac\Main AutoUpdateInit.luaq > AutoUpdateInit.lua
```

Now the fun begins.

At this point we can make certain changes by overlapping game code with our own.

To do that, we need to determine which module to override and make a copy of it.

Then, make the necessary code changes and compile the module.

Lastly, we need to inject it into the game. This can be achieved several ways.

One way is to use one of the existing TFL files in the main directory /sdcard/tap4fun/galaxylegend/Documents/ and make it load our customized module.

So this takes practice with trial and error to work correctly. Once you understand how LUA and T4F server calls work, it gets easier.

Here is an example of LUA code removing VIP limits for various actions on the client:
```
--custom_V1vip_explimit.luaq
--Original file: V1vip_explimit.lua (in the provided source code zip file)
--remove vip limits on the client
local limit = GameData.vip_exp.limit
limit.one_key_collect           = {limit = "one_key_collect"          , vip_level = 0, player_level = 0}
limit.skip_battle               = {limit = "skip_battle"              , vip_level = 0, player_level = 0}
limit.krypton_auto_compose      = {limit = "krypton_auto_compose"     , vip_level = 0, player_level = 0}
limit.krypton_auto_decompose    = {limit = "krypton_auto_decompose"   , vip_level = 0, player_level = 0}
limit.no_enhance_cd             = {limit = "no_enhance_cd"            , vip_level = 0, player_level = 0}
limit.wd_skip_battle            = {limit = "wd_skip_battle"           , vip_level = 0, player_level = 0}
limit.one_key_equip_enhance     = {limit = "one_key_equip_enhance"    , vip_level = 0, player_level = 0}
limit.mine_speed_up             = {limit = "mine_speed_up"            , vip_level = 0, player_level = 0}
limit.clear_mine_atk_cd         = {limit = "clear_mine_atk_cd"        , vip_level = 0, player_level = 0}
limit.one_key_technology_update = {limit = "one_key_technology_update", vip_level = 0, player_level = 0}
limit.remodel_speed_up          = {limit = "remodel_speed_up"         , vip_level = 0, player_level = 0}
limit.commmon_skip_battle       = {limit = "commmon_skip_battle"      , vip_level = 0, player_level = 0}
limit.ac_skip_battle            = {limit = "ac_skip_battle"           , vip_level = 0, player_level = 0}
limit.climbtower_skip_battle    = {limit = "climbtower_skip_battle"   , vip_level = 0, player_level = 0}
limit.arena_skip_battle         = {limit = "arena_skip_battle     "   , vip_level = 0, player_level = 0}
```

Needless to say, the original values were not 0's.

Let's compile this module.
```
luac -o custom_V1vip_explimit.luaq custom_V1vip_explimit.lua
```

Then, we need to encrypt it.
```
xortool custom_V1vip_explimit.luaq custom_V1vip_explimit.tfl 1
```

Now, we need to inject it via an existing module and prevent it from being overwritten.
```
Create MAIL.lua with following code:
local GameMail = LuaObjectManager:GetLuaObject("GameMail")
function GameMail.executeSave()
  return
end
ext.dofile("custom_V1vip_explimit.tfl")
```

Let's compile this other module.
```
luac -o MAIL.luaq MAIL.lua
```

Then, we also need to encrypt it.
```
xortool MAIL.luaq MAIL.tfl 1
```

Backup MAIL.lua in /sdcard/tap4fun/galaxylegend/Documents/ as MAIL.lua.bak

Copy custom_V1vip_explimit.tfl to /sdcard/tap4fun/galaxylegend/Documents/

Copy MAIL.lua to /sdcard/tap4fun/galaxylegend/Documents/

I went much further with this and scripted more than 60% of daily tasks.

Tap changes some server calls now and again, so need to verify changes are still current at every release.

Maybe I will post these customization later for you to check out.

That's it!
