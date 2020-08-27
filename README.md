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
virusfriendly@(none):~$ getent passwd nonexistantuser
nonexistantuser:*:32767:32767:CANARY:/home/SSHapendoes:/bin/false
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
**In case it's not obvious, mucking around with authenication internals can accidentailly disable your ability to log into the system, or worse allow attackers to log in. I don't recommend installing this on a production server, or in a secure environment.**

## OS Support
SSHapendoes is known to work on the following Operating Systems/Distros

Linux

* Raspbian 10
* Ubuntu 20

### How YOU can help
If you successfully install this project, create an issue letting me know what distro and version you installed it on and any installation notes.

If you're the clever type and can help with making this project more portable, feel free send a pull request.

# Installation
## Ubuntu 20
### Setting up the Build Environment
`apt install build-essential libpam0g-dev`

### Compiling
`make`

### Installing
`sudo make install`

### Configuration
#### NS Switch
Edit /etc/nsswitch and append the passwd line with canary as in the following example.

```
passwd:         files systemd canary
```

**It's very important that canary is listed last, otherwise once PAM is configured all users will be blocked.**

Additionally, any install packages that check for a user's existence before creating daemon accounts (like TOR does), will fail because of this NSSwitch Configuration. You will need to temporarily disable and later renable this configuration for such installs.

You can test the configuration with the following command

`getent passwd nonexistantuser`

which should display something like the following

`nonexistantuser:*:32767:32767:CANARY:/home/SSHapendoes:/bin/false`

#### SSHD PAM
Edit /etc/pam.d/sshd and insert the following line before any other `auth` configure lines. Often this means to insert prior to the `@include common-auth` line.

`auth       requisite    pam_canary.so`

With NS Switch and the SSHD PAM configurations updated, SSHapendoes can be tested by SSHing into the localhost with a unknown user like so:

`ssh nonexistantuser@localhost`

Supply fake passwords until SSHD kicks you out, then check the auth.log as follows:

`grep SSHapendoes auth.log`

which should show something like the following:

```
auth.log:Aug 27 17:06:43 ubuntu20server sshd[1986]: SSHapendoes Triggered user=nonexistantuser passwd=changeme rhost=127.0.0.1
auth.log:Aug 27 17:06:47 ubuntu20server sshd[1986]: SSHapendoes Triggered user=nonexistantuser passwd=badpasswd rhost=127.0.0.1
auth.log:Aug 27 17:06:50 ubuntu20server sshd[1986]: SSHapendoes Triggered user=nonexistantuser passwd=letmein rhost=127.0.0.1
```

An example modified config file using the default sshd pam configuration on Ubuntu 20

```
# PAM configuration for the Secure Shell service

auth       requisite    pam_canary.so

# Standard Un*x authentication.
@include common-auth

# Disallow non-root logins when /etc/nologin exists.
account    required     pam_nologin.so

# Uncomment and edit /etc/security/access.conf if you need to set complex
# access limits that are hard to express in sshd_config.
# account  required     pam_access.so

# Standard Un*x authorization.
@include common-account

# SELinux needs to be the first session rule.  This ensures that any
# lingering context has been cleared.  Without this it is possible that a
# module could execute code in the wrong domain.
session [success=ok ignore=ignore module_unknown=ignore default=bad]        pam_selinux.so close

# Set the loginuid process attribute.
session    required     pam_loginuid.so

# Create a new session keyring.
session    optional     pam_keyinit.so force revoke

# Standard Un*x session setup and teardown.
@include common-session

# Print the message of the day upon successful login.
# This includes a dynamically generated part from /run/motd.dynamic
# and a static (admin-editable) part from /etc/motd.
session    optional     pam_motd.so  motd=/run/motd.dynamic
session    optional     pam_motd.so noupdate

# Print the status of the user's mailbox upon successful login.
session    optional     pam_mail.so standard noenv # [1]

# Set up user limits from /etc/security/limits.conf.
session    required     pam_limits.so

# Read environment variables from /etc/environment and
# /etc/security/pam_env.conf.
session    required     pam_env.so # [1]
# In Debian 4.0 (etch), locale-related environment variables were moved to
# /etc/default/locale, so read that as well.
session    required     pam_env.so user_readenv=1 envfile=/etc/default/locale

# SELinux needs to intervene at login time to ensure that the process starts
# in the proper default security context.  Only sessions which are intended
# to run in the user's context should be run after this.
session [success=ok ignore=ignore module_unknown=ignore default=bad]        pam_selinux.so open

# Standard Un*x password updating.
@include common-password
```

#### SSHD (Optional)

Edit /etc/ssh/sshd_config

If you don't want SSHapendoes/SSHD to resolve the attacker's IP. Set `UseDNS no`.

**Red flag warnings all over the place here. Be sure you know what you're doing before proceeding further. Don't be mad at me if you kill your box...or worse**

Most attackers target the root account. To collect the passwords for these attacks, you need to first disable the root password. This is the default configuation on Ubuntu systems, but it is best to double check.

Then enable Root Login in the SSHD Config as follows.

`PermitRootLogin yes`

# Future Plans

* SSHapendoes Docker Images
* Including other services, like Samba
* Adding SSHapendoes support in [SSH-Rankings](https://github.com/pronto/SSH-Ranking)

If you think this project could use additional features, please submit a feature request.
