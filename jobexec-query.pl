#!/usr/bin/perl -w

use strict;
use warnings;

my $JOBEXECDIR="/var/tmp/jobexec";

# --- MAIN ---
#

if (open(my $fh, '<', "/etc/jobexec/jobexecdir")) {
   $JOBEXECDIR=<$fh>;
   chomp $JOBEXECDIR;
   close $fh;
}

if (open(my $fh, '<', "$ENV{HOME}/.jobexec/jobexecdir")) {
   $JOBEXECDIR=<$fh>;
   chomp $JOBEXECDIR;
   close $fh;
}

 my @dirs;
 my $timeout = 0;
 if (scalar @ARGV == 0) {
    opendir(my $dh, $JOBEXECDIR) || die "couldn't open $JOBEXECDIR";
    @dirs = grep { /^[^\.]/ && -d "$JOBEXECDIR/$_"} readdir $dh;
    closedir $dh;
 } else {
    my $id = $ARGV[0];
    $id =~ s/\.\///g;
    my $dirid = "$JOBEXECDIR/$id";
    if (not -d $dirid) {
       print "\n  FATAL: jobexec output $dirid not found \n";
       print "\n  $0: Get last run and status from jobs executed via jobexec 
  Usage:

      $0 [jobid] [timeout]
      
  Where jobid is the directory inside /var/tmp/jobexec where the job is being
  logged, and [timeout] is the time in seconds to consider as max time since last run\n\n";
       exit(3);
    }
    push @dirs, $id; 
    $timeout = $ARGV[1] if (defined $ARGV[1] and $ARGV[1] > 0);
 }

 my $stat="UNKNOWN";
 my $ago=0;
 my $runningtime=-1;
 foreach(@dirs) {
    print "$_\t";
    my $last=lastrun($_);
    if (defined $last) { 
       $stat=status("$JOBEXECDIR/$_/$last");
       $runningtime=runningtime("$JOBEXECDIR/$_/$last");
       $ago=time() - $last;
    } else { 
       $stat = "UNKNOWN";
       $runningtime = -1;
    };
    print $stat;
    print "\tl:", $ago  if ($stat ne "UNKNOWN");
    print "\tt:", $runningtime if ($runningtime > 0);
    print "\tTIMEOUT" if ($timeout > 0 && $ago > $timeout);
    print "\n";
 }

 exit 1 if ($timeout > 0 && $ago > $timeout);
 exit 2 if ($stat eq "UNKNOWN" || $stat eq "BAD_EXIT");
 exit 0;


#-------------

sub lastrun {
   my $id=pop;
   my $dir="$JOBEXECDIR/$id";
  
   opendir(my $dh, $dir) || die "couldn't open $JOBEXECDIR";
   my @dirs = grep { /^[^\.]/ && -d "$dir/$_"} readdir $dh;
   my @sdir = sort @dirs ;
   my $last = pop @sdir;
   return $last;
}


sub runningtime {
   my $dir=pop;
   open(my $fh, '<', "$dir/exec_time")
        or return -1;
    
   my $row = <$fh>;
   close $fh;
   chomp $row;
   return $row;
}

sub status {
   my $dir=pop;

   return "RUNNING" if (-f "$dir/running"); 
   return "BAD_EXIT" if (-f "$dir/bad_exit_status"); 
   return "OK" if (-f "$dir/finished"); 
# neither running nor finished.. race condition? wait a sec and try again
   sleep 1;
   return "RUNNING" if (-f "$dir/running"); 
   return "BAD_EXIT" if (-f "$dir/bad_exit_status"); 
   return "OK" if (-f "$dir/finished"); 
# if nothing showed up, the dir is broken
   return "UNKNOWN";
}


