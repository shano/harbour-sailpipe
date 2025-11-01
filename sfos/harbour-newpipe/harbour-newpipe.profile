# -*- mode: sh -*-

# Firejail profile for /usr/bin/harbour-newpipe

# x-sailjail-translation-catalog = harbour-newpipe
# x-sailjail-translation-key-description = permission-la-harbour-newpipe-transfer
# x-sailjail-description = NewPipe Transfer Engine access
# x-sailjail-translation-key-long-description = permission-la-newpipe-transfer_description
# x-sailjail-long-description = Allow downloads to be cancelled by the Transfer Engine

### PERMISSIONS
# x-sailjail-permission = Internet
# x-sailjail-permission = Audio
# x-sailjail-permission = Downloads
# x-sailjail-permission = WebView

dbus-user.own uk.co.flypig.newpipe
