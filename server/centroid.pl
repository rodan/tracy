#!/usr/bin/perl

#
#  Author:          Petre Rodan <petre.rodan@simplex.ro>
#  Available from:  https://github.com/rodan/tracy
#  License:         GNU GPLv3
#


use strict;
use warnings;
use Getopt::Long;
use Math::Trig;
use DBI;

use feature ':5.10';

my $database = undef;
my $database_cell = "/var/lib/tracking/cell_cache.db";
my $verbose  = 0;
my $dryrun = 0;

GetOptions(
    "db=s"  => \$database,
    "verbose" => \$verbose,
    "dryrun"  => \$dryrun,
);

if (!$database) {
    die 'ERROR: no such db file';
} 

my $tr;

my @cell;

my $row;
my $sth;
my $sth_update;
my $dsn = "DBI:SQLite:database=$database";
my $username = undef;
my $password = undef;
my $dbh = DBI->connect($dsn, $username, $password, { RaiseError => 1 }) or die $DBI::errstr;
my $dsn_cell = "DBI:SQLite:database=$database_cell";
my $dbh_cell = DBI->connect($dsn_cell, $username, $password, { RaiseError => 1 }) or die $DBI::errstr;

sub cell_tower_pos {
    my ($cell_id, $lac, $mcc, $mnc) = @_;

    # try to get the values from local cache db
    my $sth = $dbh_cell->prepare('SELECT lat, long FROM live where cell_id = "' . $cell_id . '" and lac = "' . $lac . '" and mnc = "' . $mnc . '" and mcc = "' . $mcc . '";');
    $sth->execute();

    my $row_cell = $sth->fetchrow_arrayref();

    if (!$row_cell) {
        my $coord_cell = `python ./cellpos.py $cell_id $lac $mcc $mnc`;
        $coord_cell =~ s/[()]//g;
        (@$row_cell[0], @$row_cell[1]) = split(', ', $coord_cell, 2);

        if ($row_cell) {
            # insert into local cache
            my $u = time;
            my $sth = $dbh_cell->prepare('INSERT INTO live (date, cell_id, lac, mnc, mcc, lat, long) VALUES ("' . $u . '", "' . $cell_id . '", "' . $lac . '", "' . $mnc . '", "' . $mcc .'", "' . @$row_cell[0] . '", "' . @$row_cell[1] . '")');
            $sth->execute();
        }
    }

    return (@$row_cell[0], @$row_cell[1]);
}

sub avg_coord {
    my $elem = ($tr->{'payload'} & 0x7);

    my $X = 0;
    my $Y = 0;
    my $Z = 0;

    for (my $i = 0; $i < $elem; $i++) {

        my $lat = deg2rad($cell[$i]->{'latitude'});
        my $lon = deg2rad($cell[$i]->{'longitude'});

        my $x = cos($lat) * cos($lon);
        my $y = cos($lat) * sin($lon);
        my $z = sin($lat);

        $X += $x;
        $Y += $y;
        $Z += $z;
    }

    $X /= $elem;
    $Y /= $elem;
    $Z /= $elem;

    my $Lon = atan2($Y, $X);
    my $Hyp = sqrt($X * $X + $Y * $Y);
    my $Lat = atan2($Z, $Hyp);

    return (rad2deg($Lat), rad2deg($Lon));
}



if ( -z "$database_cell" ) {
    $dbh_cell->do('CREATE TABLE live (row_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                 date DATE NOT NULL,
                                 hits INTEGER,
                                 cell_id INTEGER NOT NULL,
                                 lac INTEGER NOT NULL,
                                 mnc INTEGER NOT NULL,
                                 mcc INTEGER NOT NULL,
                                 lat FLOAT NOT NULL,
                                 long FLOAT NOT NULL
                                )');
} 


$sth = $dbh->prepare('SELECT row_id, payload, calc_status, c0_rxl, c0_mcc, c0_mnc, c0_cellid, c0_lac, c1_rxl, c1_mcc, c1_mnc, c1_cellid, c1_lac, c2_rxl, c2_mcc, c2_mnc, c2_cellid, c2_lac, c3_rxl, c3_mcc, c3_mnc, c3_cellid, c3_lac FROM live WHERE calc_status IS NULL');
$sth->execute();

my @list = ();

while ($row = $sth->fetchrow_arrayref()) {
    ($tr->{'row_id'}, $tr->{'payload'}, $tr->{'calc_status'},
        $cell[0]->{'rxl'}, $cell[0]->{'mcc'}, $cell[0]->{'mnc'}, $cell[0]->{'id'}, $cell[0]->{'lac'}, 
        $cell[1]->{'rxl'}, $cell[1]->{'mcc'}, $cell[1]->{'mnc'}, $cell[1]->{'id'}, $cell[1]->{'lac'}, 
        $cell[2]->{'rxl'}, $cell[2]->{'mcc'}, $cell[2]->{'mnc'}, $cell[2]->{'id'}, $cell[2]->{'lac'}, 
        $cell[3]->{'rxl'}, $cell[3]->{'mcc'}, $cell[3]->{'mnc'}, $cell[3]->{'id'}, $cell[3]->{'lac'}) = @$row;

    print $tr->{'row_id'} . ' {';

    for (my $i = 0; $i < ($tr->{'payload'} & 0x7); $i++) {
        ($cell[$i]->{'latitude'}, $cell[$i]->{'longitude'}) = cell_tower_pos($cell[$i]->{'id'}, $cell[$i]->{'lac'}, $cell[$i]->{'mcc'}, $cell[$i]->{'mnc'});
        print '+';
    }
        ($tr->{'avg_latitude'}, $tr->{'avg_longitude'}) = avg_coord();
        print '}, ';
        $sth_update = $dbh->prepare('UPDATE live SET c0_latitude = "' . $cell[0]->{'latitude'} . '", c0_longitude = "' . $cell[0]->{'longitude'} .
                '", c1_latitude = "' . $cell[1]->{'latitude'} . '", c1_longitude = "' . $cell[1]->{'longitude'} .
                '", c2_latitude = "' . $cell[2]->{'latitude'} . '", c2_longitude = "' . $cell[2]->{'longitude'} .
                '", c3_latitude = "' . $cell[3]->{'latitude'} . '", c3_longitude = "' . $cell[3]->{'longitude'} .
                '", avg_latitude = "' . $tr->{'avg_latitude'} . '", avg_longitude = "' . $tr->{'avg_longitude'} .
                '", calc_status = "' . 0x1 .
                '" WHERE row_id = ' . $tr->{'row_id'});
        $sth_update->execute();
}

$sth->finish();
if ($sth_update) {
    $sth_update->finish();
}
$dbh->disconnect();
$dbh_cell->disconnect();



