#!/usr/bin/perl

use strict;
use CGI;
use CGI qw(:standard);
use DBI;

print "Content-Type: text/html\n\n";

my $WF = new CGI;
my $imei = $WF->param('imei');
my $loc = $WF->param('loc');
my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime(time);
my $timestamp = sprintf "%4d-%02d-%02d %02d:%02d:%02d", 1900+$year,$mon+1,$mday,$hour,$min,$sec;

my $database = "/var/lib/tracking/$imei.db";
my $dsn = "DBI:SQLite:database=$database";
my $username = undef;
my $password = undef;
my $dbh = DBI->connect($dsn, $username, $password, { RaiseError => 1 });

if ( -z "$database" ) {
    $dbh->do("CREATE TABLE live (row_id INTEGER PRIMARY KEY AUTOINCREMENT,
                                 imei VARCHAR(20) NOT NULL,
                                 date DATE NOT NULL,
                                 loc VARCHAR(20) NOT NULL,
                                 fixtime INTEGER
                                )");
}


my $stmt = qq(INSERT INTO live (imei, date, loc)
      VALUES ("$imei", "$timestamp", "$loc" ));
my $rv = $dbh->do($stmt) or die $DBI::errstr;

$dbh->disconnect();

print "SAVED\r\n";


