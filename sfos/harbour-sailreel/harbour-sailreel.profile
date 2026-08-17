# -*- mode: sh -*-

# Firejail profile for /usr/bin/harbour-sailreel

# x-sailjail-translation-catalog = harbour-sailreel
# x-sailjail-translation-key-description = permission-la-harbour-sailreel-transfer
# x-sailjail-description = sailreel Transfer Engine access
# x-sailjail-translation-key-long-description = permission-la-sailreel-transfer_description
# x-sailjail-long-description = Allow downloads to be cancelled by the Transfer Engine

### PERMISSIONS
# x-sailjail-permission = Internet
# x-sailjail-permission = Audio
# x-sailjail-permission = Downloads
# x-sailjail-permission = Videos
# x-sailjail-permission = WebView

dbus-user.own io.github.shano.harbour-sailreel
