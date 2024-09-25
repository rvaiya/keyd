# Keyd
## From Source
### SystemD
    git clone https://github.com/danilanosikov/keyd
    cd keyd
    make && sudo make install
    sudo systemctl enable keyd && sudo systemctl start keyd
### OpenRC
	git clone https://github.com/danilanosikov/keyd
    cd keyd
    make && doas make install
    doas rc-update add keyd && doas rc-service keyd start
## Quickstart

	1. Install and start keyd (e.g `sudo systemctl enable keyd` or `doas rc-service keyd start`)
	
	2. Put the following in `/etc/keyd/default.conf`:
	
	```
	[ids]
	
	*
	
	[main]
	
	# Maps capslock to escape when pressed and control when held.
	capslock = overload(control, esc)
	
	# Remaps the escape key to capslock
	esc = capslock
	```
