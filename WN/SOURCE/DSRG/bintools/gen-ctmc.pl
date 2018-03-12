#!/usr/bin/env perl

use warnings;
use strict;


foreach (<>)
{
  
  if (/fontsize=7, label=\"/) {
      chomp $_ ;
      print $_ ."\\n";
  }
  elsif (/|\d+|=\d+/) {
      chomp $_ ;
      print $_ ;
  }
  else{
      print $_;
  }
 
}

