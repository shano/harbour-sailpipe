# -*- mode: sh -*-

# Firejail profile for /usr/bin/harbour-sailpipe

# x-sailjail-translation-catalog = harbour-sailpipe
# x-sailjail-translation-key-description = permission-la-harbour-sailpipe-transfer
# x-sailjail-description = sailpipe Transfer Engine access
# x-sailjail-translation-key-long-description = permission-la-sailpipe-transfer_description
# x-sailjail-long-description = Allow downloads to be cancelled by the Transfer Engine

### PERMISSIONS
# x-sailjail-permission = Internet
# x-sailjail-permission = Audio
# x-sailjail-permission = Downloads
# x-sailjail-permission = WebView

dbus-user.own uk.co.flypig.sailpipe
