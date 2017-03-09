### Set up no focus

Add to `/lib/udev/rules.d/99-systemd.rules`:

        KERNEL=="video0", SYMLINK="video0", TAG+="systemd"
        KERNEL=="video1", SYMLINK="video1", TAG+="systemd"
