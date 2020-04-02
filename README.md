![SSHapendoes' Logo](https://github.com/VirusFriendly/SSHapendoes/blob/master/assets/SSHapendoes-logo.png)
### Capture passwords of login attempts for non-existent and disabled accounts.

SSHapendoes turns any Linux host with an SSH port into a medium-interaction honeypot, by spoofing the existence of Canary Accounts in a similar manner to AD Honey Accounts.

Anyone with an SSH port open to the Internet is constantly being attacked by threat actors attempting to brute-force root and other accounts. Metadata such as a list of targeted users can be used as behavioral signatures to profile threat actors as they attack from different hosts. Password lists is another key piece of metadata, but usually attempted passwords are not logged. This tool exposes the passwords attempted by attackers without exposing passwords of legitimite user accounts.

## Background

While studying attacker behavior using [SSH-Ranking](https://github.com/pronto/SSH-Ranking), I noticed that different attackers attack different lists of users, and that these userlists could be used identitify attackers even if they use different IP's. I became curious of what could be learned by studying the passwords they used. Additionally, I have a side project of writing John the Ripper [word mangling rules](https://github.com/maetrics/john-scripts), for which harvesting actual attacker passwords is useful.

My first step to gathering passwords was to build a custom PAM module. (Editorial/Rant: I find that PAM is one of those things that most admins know what it stands for, and what it does, but often don't understand the inner workerings and probably wouldn't be able to identify a malicious PAM module. I highly recommend creating your own custom PAM module to be more enlightened with the power they contain. In the future I'll likely release various malicious PAM modules to demonstrate this. For use in CTFs and pen-tests of course.)

However, even with a custom PAM module, OpenSSH don't send passwords to PAM for accounts that don't exist. Instead it sends #010#012#015#177INCORRECT. My guess is this is to prevent attackers learning which accounts are valid by using timing attacks. To work around this issue, I created a custom NSS module that spoofs all accounts.

The result:

```
Mar 31 08:24:02 (none) sshd[1382]: SSHapendoes Triggered user=root passwd=tslinux rhost=115.230.127.61
Mar 31 08:24:02 (none) sshd[1382]: SSHapendoes Triggered user=root passwd=kodiak rhost=115.230.127.61
Mar 31 08:24:09 (none) sshd[1386]: SSHapendoes Triggered user=root passwd=PASSW0RD rhost=115.230.127.61
Mar 31 08:24:10 (none) sshd[1386]: SSHapendoes Triggered user=root passwd=qwerty15 rhost=115.230.127.61
Mar 31 08:24:10 (none) sshd[1386]: SSHapendoes Triggered user=root passwd=k4hvdq9tj9 rhost=115.230.127.61
Mar 31 08:24:21 (none) sshd[1388]: SSHapendoes Triggered user=root passwd=itadmin rhost=115.230.127.61
Mar 31 08:24:21 (none) sshd[1388]: SSHapendoes Triggered user=root passwd=picasso rhost=115.230.127.61
Mar 31 08:24:21 (none) sshd[1388]: SSHapendoes Triggered user=root passwd=needhouse rhost=115.230.127.61
Mar 31 08:24:30 (none) sshd[1390]: SSHapendoes Triggered user=root passwd=cracker88 rhost=115.230.127.61
```

## How it works

After getpwnam() checks the legitimate passwd databases for users, it will check nss_canary which will spoof any user request it recieves. These user requests will have "CANARY" in the gecos field. Legitimate users will not be affected by this, as NSSwitch will be configured to use CANARY as last resort.

For example:

```
gomi@(none):~$ getent passwd nonexistantuser
nonexistantuser:stantuser:32767:32767:CANARY:/home/SSHapendoes:/bin/false
```

This "CANARY" gecos field will indicate to the SSHapendoes PAM module that it's a spoofed account, and log the attempt along with the password used.

The SSHapendoes PAM module will also log attempts for accounts with no password hash (as determined by NSSwitch).

For example, these accounts have no password hashes:

```
root:*:16112:0:99999:7:::
daemon:*:16112:0:99999:7:::
bin:*:16112:0:99999:7:::
sys:*:16112:0:99999:7:::
sync:*:16112:0:99999:7:::
games:*:16112:0:99999:7:::
```

If the account doesn't have "CANARY" in the gecos field, and it has a hash in the shadow file (as determined by NSSwitch), the SSHapendoes PAM module will return a success without logging anything, allowing PAM to continue through its normal list of modules. Thus allowing normal user logins via passwords or ssh-keys, and without logging the passwords of legitimate users.

## Warning
In case it's not obvious, mucking around with authenication internals can accidentailly disable your ability to log into the system, or worse allow attackers to log in. I don't recommend installing this on a production server, or in a secure environment.

## OS Support
At this stage of the project it's a "works on my system" support. If your system isn't my system, and it shouldn't be, then installation may be different for you.

I will attempt support different Linux distos and perhaps BSD/MAC.


### How YOU can help
If you successfully install this project, create an issue letting me know what distro and version you installed it on, and any installation instructions that differ from what I have written below.

If you're the clever type and can help with making this project more portable, feel free send a pull request.

## Installation
To compile:

`make`

To install, first check the Makefile to ensure that the destination directories are correct. Then:

`sudo make install`

Next edit /etc/nsswitch and edit the passwd line to look like the following:

```
passwd:         compat canary
```

It's very important that canary is listed last, otherwise once PAM is configured all users will be blocked.

Next edit /etc/pam.d/sshd to look something like this, making sure that pam_canary.so proceeds the auctual authentication lines, but comes after the required modules.

```
# PAM configuration for the Secure Shell service

# Read environment variables from /etc/environment and
# /etc/security/pam_env.conf.
auth       required     pam_env.so # [1]
# In Debian 4.0 (etch), locale-related environment variables were moved to
# /etc/default/locale, so read that as well.
auth       required     pam_env.so envfile=/etc/default/locale
auth       requisite    pam_canary.so

# Standard Un*x authentication.
@include common-auth
```

Also, if you don't want SSHapendoes to resolve the attacker's IP. Edit your sshd.conf and set `UseDNS no`.

**Red flag warnings all over the place here. Be sure you know what you're doing before proceeding further. Don't be mad at me if you kill your box...or worse**

Lastly, most attackers target the root account. To get the passwords for these attacks, you need to first disable the root password. Then edit /etc/ssh/sshd_config and enable Root Login

`PermitRootLogin yes`

## Future Plans

I'm currently forking [SSH-Rankings](https://github.com/maetrics/SSH-Ranking) to add support for parsing SSHapendoes logs and eventually add threat actor behavior profiles using analysis of various metadata signatures.

If you think this project could use additional features, see if the feature is a better fit for SSH-Rankings. If you find that it's a good fit for either project, submit a feature request.

I'm also going to look into support for other services such as Samba
