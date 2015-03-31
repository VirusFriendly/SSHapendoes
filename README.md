# CatchAll
Capture passwords of login attempts for non-existent and disabled accounts.

## Background
If you have an open SSH port exposed to the Internet, someone is trying to brute force the log into your system right now.

If you're interested in monitoring who is attacking your ssh logins, I suggest checking out https://github.com/pronto/SSH-Ranking (@sshbrute).

While studying attacker behavior using SSH-Ranking, I have noticed that different attackers attack different lists of users, and that these userlists could be used identitify attackers even if they use different IP's. I became curious of what could be learned by studying the passwords they used. This information would help me further "tag" attackers, and help with my john-the-ripper rulesets.

My first step to gathering passwords was to build a custom PAM module. (Editorial/Rant: I find that PAM is one of those things that most admins know what it stands for, and what it does, but often don't understand the inner workerings and probably wouldn't be able to identify a malicious PAM module. I highly recommend creating your own custom PAM module to be more enlightened with the power they contain. In the future I'll likely release various malicious PAM modules to demonstrate this. For use in CTFs and pen-tests of course.)

However, even with a custom PAM module, OpenSSH don't send passwords to PAM for accounts that don't exist. Instead it sends #010#012#015#177INCORRECT. My guess is this is to prevent attackers learning which accounts are valid by using timing attacks. To work around this issue, I created a custom NSS module that spoofs all accounts.

The result:

```
Mar 31 08:24:02 (none) sshd[1382]: CatchAll Triggered user=root passwd=tslinux rhost=115.230.127.61
Mar 31 08:24:02 (none) sshd[1382]: CatchAll Triggered user=root passwd=kodiak rhost=115.230.127.61
Mar 31 08:24:09 (none) sshd[1386]: CatchAll Triggered user=root passwd=PASSW0RD rhost=115.230.127.61
Mar 31 08:24:10 (none) sshd[1386]: CatchAll Triggered user=root passwd=qwerty15 rhost=115.230.127.61
Mar 31 08:24:10 (none) sshd[1386]: CatchAll Triggered user=root passwd=k4hvdq9tj9 rhost=115.230.127.61
Mar 31 08:24:21 (none) sshd[1388]: CatchAll Triggered user=root passwd=itadmin rhost=115.230.127.61
Mar 31 08:24:21 (none) sshd[1388]: CatchAll Triggered user=root passwd=picasso rhost=115.230.127.61
Mar 31 08:24:21 (none) sshd[1388]: CatchAll Triggered user=root passwd=needhouse rhost=115.230.127.61
Mar 31 08:24:30 (none) sshd[1390]: CatchAll Triggered user=root passwd=cracker88 rhost=115.230.127.61
```

## How it works

After getpwnam() checks the legitimate passwd databases for users, it will check nss_catchall which will spoof any user request it recieves. These user requests will have "CatchAll" in the gecos field.

For example:

```
gomi@(none):~$ getent passwd nonexistantuser
nonexistantuser:stantuser:32767:32767:CatchAll:/home/catchall:/bin/false
```

This "CatchAll" gecos field will indicate to the CatchAll PAM module that it's a spoofed account, and log the attempt along with the password used.

The CatchAll PAM module will also log attempts for accounts with no password hash.

For example, these accounts have no password hashes:

```
root:*:16112:0:99999:7:::
daemon:*:16112:0:99999:7:::
bin:*:16112:0:99999:7:::
sys:*:16112:0:99999:7:::
sync:*:16112:0:99999:7:::
games:*:16112:0:99999:7:::
```

## Warning
In case it's not obvious, mucking around with authenication internals can accidentailly disable your ability to log into the system, or worse allow attackers to log in. I don't recommend installing this on a production server, or in a secure environment.

## Installation
At this stage of the project it's a "works on my system" support. If your system isn't my system, and it shouldn't be, then installation may be different for you. If you're the clever type and can help with making this project more portable, feel free send a pull request.

To compile:

`make`

To install, first check the Makefile to ensure that the destination directories are correct. Then:

`sudo make install`

Next edit /etc/nsswitch and edit the passwd line to look like the following:

```
passwd:         compat catchall
```

It's very important that catchall is listed last, otherwise once PAM is configured all users will be blocked.

Next edit /etc/pam.d/sshd to look something like this, making sure that pam_catchall.so proceeds the auctual authentication lines, but comes after the required modules.

```
# PAM configuration for the Secure Shell service

# Read environment variables from /etc/environment and
# /etc/security/pam_env.conf.
auth       required     pam_env.so # [1]
# In Debian 4.0 (etch), locale-related environment variables were moved to
# /etc/default/locale, so read that as well.
auth       required     pam_env.so envfile=/etc/default/locale
auth       requisite    pam_catchall.so

# Standard Un*x authentication.
@include common-auth
```
*Red flag warnings all over the place here. Be sure you know what you're doing before proceeding further. Don't be mad at me if you kill your box...or worse*

Lastly, most attackers target the root account. To get the passwords for these attacks, you need to first disable the root password. Then edit /etc/ssh/sshd_config and enable Root Login

`PermitRootLogin yes`
