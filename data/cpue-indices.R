#!/usr/bin/Rscript

# A R script to document and dump CPUE indices

# Maldives Pole and Line CPUE
#
# Use the "Standardised FAD" index from Table 1 of Sharma et al (2013)
# Sharma, R, Geehan, J., Adam, M.S. (2013) Maldives Skipjack Pole and Line Fishery Catch Rate Standardization 2004-2011 IOTC-2013-WPTT-15-XX
# File: IOTC-2013-WPTT-XX - Maldives SKJ CPUE Revisited v5.pdf
cat(
"year	quarter	index
2004	1	1.22
2004	2	1.09
2004	3	1.2
2004	4	1.41
2005	1	1.44
2005	2	1.28
2005	3	1.41
2005	4	1.66
2006	1	1.4
2006	2	1.24
2006	3	1.38
2006	4	1.62
2007	1	1
2007	2	0.88
2007	3	0.98
2007	4	1.15
2008	1	0.92
2008	2	0.81
2008	3	0.9
2008	4	1.06
2009	1	0.83
2009	2	0.73
2009	3	0.81
2009	4	0.95
2010	1	0.66
2010	2	0.58
2010	3	0.64
2010	4	0.76
2011	1	0.49
2011	2	0.44
2011	3	0.49
2011	4	0.57
",
file = "processed-data/m-pl-cpue.txt"
)

# Purse seine CPUE
#
# Use the "PSFS" index as used by Sharma (2012) SKJ assessment
# File: 2012CPUEindicesSkipjack.xlsx (provided by Rishi)
cat(
"year	index
1991	0.366168128
1992	0.804538422
1993	1.010830325
1994	1.717380093
1995	1.21196493
1996	0.861268695
1997	0.582774626
1998	0.665291387
1999	1.768953069
2000	1.258380609
2001	1.010830325
2002	1.062403301
2003	0.825167612
2004	0.990201135
2005	1.990716864
2006	1.495616297
2007	0.892212481
2008	0.598246519
2009	0.469314079
2010	0.417741104
",
file = "processed-data/w-ps-cpue.txt"
)