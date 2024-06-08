# pass-secrets
CLI Application that provides the Freedesktop Secret Service using Pass as its backend!

## Status
Currently working to store secrets with protonmail-bridge. I have not done tests with other applications; if it doesn't work, please open an issue!

## How to build
pass-secrets requires pass and sdbus-c++ to be installed on your machine. Rapidjson and nanoid are included in the repo.

```
cd pass-secrets
mkdir build
cd build
cmake ..
make
```

You can then install with `sudo make install` and uninstall with `sudo xargs rm < install_manifest.txt`. This will by default install a systemd user service that can be dbus-activated when enabled.

```
sudo make install
systemctl --user enable pass-secrets
```

Alternatively, you can run it in the background in your bashrc/xinitrc.
```
...
/usr/local/bin/pass-secrets &
...
```

If you're running pass-secrets on a BSD, or possibly a non-systemd Linux system, you'll need to start a DBus session bus before starting pass-secrets. For instance, your ~/.bash_profile could contain:

```
if ! pgrep -qf -U ${USER:-$(id -u)} dbus.\*--session; then
	dbus-daemon --session --fork --address=unix:runtime=yes 2>/dev/null
	export DBUS_SESSION_BUS_ADDRESS=unix:path=$XDG_RUNTIME_DIR/bus
fi
```

See [this issue](https://github.com/nullobsi/pass-secrets/issues/11) for more details.
