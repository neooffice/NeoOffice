# Extracts the hotspot coordinates for Win32 formatted .cur files.
#
# Requires Imager perl module:
#	http://search.cpan.org/dist/Imager/
#
# Edward Peterlin
# 6/4/07

use Imager;

$img = Imager->new;

foreach $cur (@ARGV) {
	$img->read(file=>$cur);
	$x=$img->tags(name=>'cur_hotspotx');
	$y=$img->tags(name=>'cur_hotspoty');
	print "$cur: hotspotx: $x hotspoty: $y\n";
}