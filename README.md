BitchX 1.2.1 README
===================

BitchX is a free software text-based IRC (Internet Relay Chat) client for 
UNIX-like systems, originally based on ircII and heavily influenced by
EPIC.  This is the release notes for the 1.2.1 release.

I'd like to send out a special thanks to all those who've contributed in
a large or small way to the BitchX 1.2.1 release, whether through bug reports,
patches, testing, coding, packaging, updating documentation, maintaining 
IRC channels or websites, promotion or just keeping the faith:

flashback, Xavier, panasync, dialtone, sideshow, hop, Ancient, fudd, tau,
dan408, cpet, jeezy, zimzum, t3gah, darkfires, gauze, madsage, snadge, 
packet, nyet, brabes, nenolod, jdhore, riderplus, DJ, oxy, ncopa.

This release has primarily focussed on fixing a lage number of bugs and 
updating the code to suit a modern environment - the full list of changes is
in the Changelog.  Notable changes in the 1.2.1 release that are particularly
user-visible or of interest to scripters are explained below.

Instructions on building and installing the client are in the INSTALL file,
and licensing info is found in the COPYRIGHT file.

The BitchX 1.2 series is dedicated to void.  We'll miss you, man.

    - caf

/NAMES and /SCAN
----------------

The formatting of /NAMES and /SCAN output has been substantially reworked.
The old /FSETs NAMES_BOTCOLOR, NAMES_FRIENDCOLOR, NAMES_NICKCOLOR,
NAMES_OPCOLOR, NAMES_SHITCOLOR and NAMES_VOICECOLOR have been replaced
with these new /FSETs:

....................NAMES_NICK %B$[10]0
................NAMES_NICK_BOT %G$[10]0
.............NAMES_NICK_FRIEND %Y$[10]0
.................NAMES_NICK_ME %W$[10]0
...............NAMES_NICK_SHIT %R$[10]0
....................NAMES_USER %K[ %n$1-%K]
.............NAMES_USER_CHANOP %K[%C$0%n$1-%K]
..............NAMES_USER_IRCOP %K[%R$0%n$1-%K]
..............NAMES_USER_VOICE %K[%M$0%n$1-%K]

The NAMES_NICK formats control how the nick itself is displayed, depending
on the status of the nick as recognised by BitchX (the priority order is
NAMES_NICK_ME > NAMES_NICK_BOT > NAMES_NICK_FRIEND > NAMES_NICK_SHIT >
NAMES_NICK).  The NAMES_USER formats control how the overall entry appears in
the /NAMES or /SCAN line, depending on the channel status of the nick (the
priority order is NAMES_USER_CHANOP > NAMES_USER_VOICE > NAMES_USER_IRCOP >
NAMES_USER).

You'll need to update any scripts or custom formats that altered the old
formats.  If you just use the defaults, the main difference you'll see is
that your own nick is now shown in white, and voiced users are shown with
the '+' sent by the server instead of the 'v'.  You can go back to the old
look by setting these formats:

/FSET NAMES_NICK_ME %B$[10]0
/FSET NAMES_USER_VOICE %K[%Mv%n$1-%K]

Also, the NAMES header and NAMES_FOOTER formats are now called with $3 set
to the total number of nicks in the channel, and $4 to the mask given to
/SCAN (if any).

/SCAN now supports a new sort flag "-stat", which sorts the output by
channel status (chanops, then halfops, then voices, then the riff-raff).
This functionality was previously provided by the "scan" plugin, so this
plugin has been removed. If you want this to be the default, just set this
alias (the double-slash is necessary):

/ALIAS SCAN { //SCAN -STAT }

NEW FORMATS
-----------

In addition to the new NAMES_* formats mentioned above, these new /FSET
formats have been added:

CHANNEL_URL
USERMODE_OTHER
WHOIS_CALLERID
WHOIS_SECURE
WHOIS_LOGGEDIN

HALF-OP SUPPORT
---------------

New commands /HOP and /DEHOP have been added, to give and remove halfop
status (on servers that support it).

The scripting function $ishalfop(<nick> <channel) has been added (compatible
with EPIC), and support for halfops has been added to $channel().

The default status bar now shows your halfop status against your nick.

TCL
---

panasync OK'd releasing the tcl.c source, so this is now part of the main
client distribution.  If you want TCL support, just pass --with-tcl to
./configure.

MULTISERVER
-----------

The "last nick sent to" and "last nick received from" are now tracked
per-server, so $. $, $B and the "." and "," message targets are all
per-server.
