#!/bin/bash
# vim: noet:ts=4:sts=4:sw=4

# SPDX-License-Identifier: MIT
# Copyright © 2025 David Llewellyn-Jones

# Generate graphics at different resolutions for different devices
# 1.0 (jolla phone), 1.25 (jolla c), 1.5 (tablet), 1.75 (xperia)
ratios="1.0 1.25 1.5 1.75 2.0"

# Generate app icons
sizes="86 108 128 172 256"
for size in ${sizes}; do
	mkdir -p "./icons/${size}x${size}"
	inkscape --batch-process -o "./icons/${size}x${size}/harbour-newpipe.png" -w $size -h $size "inputs/harbour-newpipe.svg"
done

# Create the ratio directories
for ratio in ${ratios}; do
	mkdir -p "./qml/images/z${ratio}"
done

# Function for generating PNG images
function generate {
	basex=$1
	basey=$2
	names=$3
	for ratio in ${ratios}; do
		sizex=`echo "${ratio} * ${basex} / 1" | bc`
		sizey=`echo "${ratio} * ${basey} / 1" | bc`
		for name in ${names}; do
			inkscape --batch-process -o "./qml/images/z${ratio}/${name}.png" -w ${sizex} -h ${sizey} "inputs/${name}.svg"
		done
	done
}

# Generate cover action icons
#generate 32 32 "icon-cover-replay"

# Generate titles
#generate 303 86 "newpipe-title"

# Generate small icons
generate 32 32 "icon-s-media-view"

# Generate medium icons
generate 64 64 "icon-tab-about icon-tab-playlists icon-tab-video-full icon-tab-video-live icon-tab-video-short"

# Generate large icons
generate 96 96 "icon-l-replay icon-l-skip"

# Generate cover
#generate 117 133 "cover-background"
