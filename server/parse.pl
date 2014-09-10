#!/usr/bin/perl -T

# perl script that parses POST packets that 
# contain tracking information
#
#  Author:          Petre Rodan <petre.rodan@simplex.ro>
#  Available from:  https://github.com/rodan/tracy
#  License:         GNU GPLv3
#

use strict;
use warnings;

use IO::String;
use Getopt::Long;

my $filename = '/tmp/foo';
my $verbose = 0;

GetOptions ("file=s" => \$filename,
            "verbose" => \$verbose);

#my $filename = '/tmp/nTfew6fNu' ;

my $buf;
my $binstr;
my $io = IO::String->new($binstr);

# HTTP POST header
my $version; my $imei; my $settings; my $v_bat; my $v_raw; my $msg_id; my $payload_content_desc;

# first 25 bytes from the nmea_gprmc_t structure
my $year; my $month; my $day; my $hour; my $minute; my $second; my $latitude; my $longitude; my $pdop; my $speed; my $heading; my $fixtime; my $geo_distance; my $geo_bearing;

# sim900_cell_t structure 
my @cell_rxl; my @cell_mcc; my @cell_mnc; my @cell_cellid; my @cell_lac;

my $suffix;

# binmode?
open(my $fh, '<', $filename) or die "cannot open file $filename";
    {
        local $/;
        $binstr = <$fh>;
    }
close($fh);

read($io, $buf, 26);

($version, $imei, $settings, $v_bat, $v_raw, $msg_id, $payload_content_desc) =
       unpack( "S A15 S4 C", $buf );
$v_bat /= 100;
$v_raw /= 100;


#  payload_content_desc is a byte that describes what info is present in the packet
#
#  binary representation:
# 
#  | 7 | 6 | 5  | 4 | 3 | 2 | 1 | 0 | 
#                       | x   x   x | -> how many sim900.cell structures are present
#                   | x |             -> geofence data is present
#               | x |                 -> gps fix is present
#

my $cells = $payload_content_desc & 0x07;
my $geofence = $payload_content_desc & 0x08;
my $fix = $payload_content_desc & 0x10;


if ($fix) {
    read ($io, $buf, 25);
    ($year, $month, $day, $hour, $minute, $second, $latitude, $longitude, $pdop, $speed, $heading, $fixtime) = unpack( "S C5 f f S3 L" , $buf );
    $pdop /= 100.0;
    if ($geofence) {
        read ($io, $buf, 6);
        ($geo_distance, $geo_bearing) = unpack( "f S" , $buf );
    }
}

if ($cells) {
    for (my $i=0; $i < $cells; $i++) {
        read($io, $buf, 10);
        ($cell_rxl[$i], $cell_mcc[$i], $cell_mnc[$i], $cell_cellid[$i], $cell_lac[$i]) = unpack( "S5", $buf );
    }
}

read($io, $buf, 1);
$suffix = unpack( "C", $buf );

#my $pos = $io->getpos;
#print "pos $pos\r\n";

if ($verbose) {

    # display all we got
    print "header {\r\n";
    print " version:   $version\r\n";
    print " imei:      $imei\r\n";
    print " settings:  $settings\r\n";
    print " v_bat:     $v_bat\r\n";
    print " v_raw:     $v_raw\r\n";
    print " msg_id:    $msg_id\r\n";
    print " payload:   $payload_content_desc\r\n";
    print "}\r\n";

    if ($fix) {
        print " timestamp: $year.$month.$day  $hour:$minute:$second\r\n loc:  $latitude, $longitude";
        print " pdop $pdop, speed $speed knots, heading $heading deg, uptime $fixtime s\r\n";
        if ($geofence) {
            print " geofence distance $geo_distance m, bearing $geo_bearing deg\r\n";
        } else {
            print "geofence data not present\r\n";
        }
    } else {
        print "gps fix not present\r\n";
    }

    if ($cells) {
        print "$cells cells present:\r\n";
        for (my $i=0; $i < $cells; $i++) {
            print "cell #$i {\r\n";
            print " rxl        $cell_rxl[$i]\r\n";
            print " mcc        $cell_mcc[$i]\r\n";
            print " mnc        $cell_mnc[$i]\r\n";
            print " id         $cell_cellid[$i]\r\n";
            print " lac        $cell_lac[$i]\r\n";
            print "}\r\n"
        }
    } else {
        print "no cell tower data\r\n";
    }

    if ($suffix != 0xff) {
        printf "ERROR: mangled packet\r\n";
        exit 1;
    }
}

