% TUNA: Toolkit for Underwater Noise Analysis
% Paul Barker
% June 2013

# Introduction

The TUNA project aims to provide a toolkit for the real-time acquisition and
signal processing of underwater acoustic data, implementing both existing and
novel analysis methods. It is written and maintained by Paul Barker as part of
his PhD studies at Loughborough University. It is distributed under the terms of
the GNU General Public License version 2 (see [COPYING][GPL-local] in the source 
tree or [view the license online][GPL-online]) and made available to the 
community to assist with the assessment, monitoring and mitigation of the 
adverse effects of anthropogenic noise on the marine environment.

This software is currently in the early development stages and may not be 
suitable for its final intended use yet. The software has only been tested on 
Linux and support for other operating systems will be added at a later date. 
However, you are invited to help test this software and report any feedback, 
bugs or feature requests via the [issue tracker].

# Obtaining Sources

There are currently no pacakged releases of TUNA. Sources can be downloaded from 
the [official project page] but are best obtained by `git`:

	git clone https://bitbucket.org/underwater-acoustics/tuna.git

# Installation

This software has a few dependencies beyond a basic Linux system capable of 
building and running software:

- [fftw] (tested with version 3.3.3)
- [libsndfile] (tested with version 1.0.25)
- [alsa-lib] (tested with version 1.0.27.1)

On Arch Linux, this software can be installed via the command:

	pacman -S fftw libsndfile alsa-lib

The command for other Linux distros is left as an excersize for the reader.

Once the dependencies are met, the software can be build and installed via 
invocation of `make` in the root directory of the project:

	make
	make install

That's all there is to it.

[GPL-local]: COPYING
[GPL-online]:http://www.gnu.org/licenses/gpl-2.0.html
[issue tracker]: https://bitbucket.org/underwater-acoustics/tuna/issues
[fftw]: http://www.fftw.org/
[libsndfile]: http://www.mega-nerd.com/libsndfile/
[alsa-lib]: http://www.alsa-project.org/
[official project page]: https://bitbucket.org/underwater-acoustics/tuna
