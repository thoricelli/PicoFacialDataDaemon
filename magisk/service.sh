#!/system/bin/sh
until [ "$(getprop sys.boot_completed)" = "1" ]; do
    sleep 2
done

# Wait for the WIFI adapter to be online and connected.
until ip addr show wlan0 2>/dev/null | grep -q "inet "; do
    sleep 2
done

MODDIR=${0%/*}

$MODDIR/system/bin/picofacialdatadaemon