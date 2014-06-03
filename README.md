plymentry
=========

A drop-in replacement for gnupg's pinentry that uses plymouthd as frontend.

plymentry is a combination of plymouth and pinentry. On one end it acts as a
drop-in replacement for gnupg's pinentry tool and on the other it communicates
with plymouthd, the daemon that displays the boot splash on fedora and ubuntu.
With plymentry the gpg-agent can use plymouthd to query passphrases and pins
from the user.

plymentry comes with a hook for initramfs-tools and a decrypt script, which
can prepare your initrd to unlock your Luks-encrypted root partion with your
openPGP-card.

I will now describe how I use plymentry, gnupg2 and an openPGP-card to unlock
the root partition of my laptop.

BEWARE: You should really know what you are doing. You have backups of
everything, right? And if you mess up you can simply reinstall your distro with
no harm done, right? So if you mess up don't come whining, you have been warned.

## Prerequisites
So far I tested and used this only with Ubuntu 14.04. You need
* an OpenPGP-card,
* a cardreader that works with gnupg2,
* a root partition or lvm base device that is an encrypted Luks-container,
* a c++ compiler that supports lambdas (I use g++ 4.8 but >= 4.5 should be fine)
* the packages libplymouth-dev, gnupg2, gpg-agent, gpgsm, pcscd

## Build and install plymentry
Build plymentry with `make` and copy the plymentry binary to `/usr/local/bin/`.
(If I get around to I may provide an autotools build config in the future.)

## Preparing the keychain and the keyfile
The decrypt script expects a keychain at `/etc/cryptroot_keyring`. So create
the directory

    sudo mkdir /etc/cryptroot_keyring

make sure it has appropriate access rights.

    sudo chmod 700 /etc/cryptroot_keyring

and install your keyring there. The script `scripts/prepare_keyring_helper`
behaves like the gpg2 binary only that it does not touch your `~/.gnupg`
directory but instead uses `/etc/cryptroot_keyring`. So

    sudo scripts/prepare_keyring_helper --import <your pub key>
    sudo scripts/prepare_keyring_helper --card-status

should install your public key in `/etc/cryptroot_keyring/pubring.gpg` and
the secret key card-stubs in `/etc/cryptroot_keyring/secring.gpg`. It also
places `/etc/cryptroot_keyring/trustdb.gpg`.
Now create the keyfile for example like this

    head -c 256 /dev/random | gpg2 -e -r "you <your@email.address>" > keyfile.gpg

or however you see fit. Place the keyfile into `/etc/cryptroot_keyring`.

## Preparing the Luks partition
Use cryptsetup to add a new keyslot to your Luks-container where you use the
decrypted version of your keyfile.gpg as passphrase. Consult the cryptsetup
manpage for this step.

## Setting up crypttab
Your `/etc/crypttab` should look somewhat like this

    sda5_crypt UUID=<UUID of your Luks-partition> none luks,discard

Replace `none` with the full path to your keyfile
(here `/etc/cryptroot_keyring/keyfile.gpg`)
and specify the script that is needed to decrypt the keyfile.

    sda5_crypt UUID=<still the same UUID> /etc/cryptroot_keyring/keyfile.gpg luks,discard,keyscript=decrypt_gnupg2

## Building the initramfs
Okay you are nearly there. The `hooks/cryptgnupg2` script from this repo goes
to `/etc/initramfs-tools/hooks/`. It gets called when building the new initrd.
It pulls in all the tools needed at boot time (gpg2, gpg-agent, scdaemon, pcscd
and plymentry, which gets installed as pinentry, so the gpg-agent can find it).
The script `scripts/decrypt_gnupg2` goes to `/lib/cryptsetup/scripts/`. Before
you build the new initrd you may want to backup your current one to a location
from where you can place it into your boot partition (e.g. with a live-cd) in
case anything goes wrong. Now generate the new initrd with:

    sudo update-initramfs -u

If you followed all of my instructions and I did not miss anything you can
reboot now. And upon reboot plymouthd should ask you to insert your card and
then enter your pin. If not ... well good luck.

