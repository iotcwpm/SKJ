#!/usr/bin/Rscript

# A R script to document and dump CPUE indices

# Maldives Pole and Line CPUE
#
# Uses the "Model 3: Standardized FAD" index from Table 1 of Sharma et al (2014)
# Sharma, R, Geehan, J., Adam, M.S. (2014) Maldives Skipjack Pole and Line Fishery Catch Rate Standardization 2004-2012: Reconstructing Historic CPUE till 1985. IOTC-2014-WPTT-15-XX
cat(
"year	quarter	index
2004	1	1.18
2004	2	1.05
2004	3	1.17
2004	4	1.44
2005	1	1.33
2005	2	1.19
2005	3	1.31
2005	4	1.63
2006	1	1.4
2006	2	1.27
2006	3	1.4
2006	4	1.73
2007	1	0.99
2007	2	0.89
2007	3	0.99
2007	4	1.22
2008	1	0.98
2008	2	0.88
2008	3	0.98
2008	4	1.23
2009	1	0.92
2009	2	0.83
2009	3	0.92
2009	4	1.12
2010	1	0.74
2010	2	0.66
2010	3	0.73
2010	4	0.91
2011	1	0.57
2011	2	0.52
2011	3	0.57
2011	4	0.7
2012	1	0.62
2012	2	0.57
2012	3	0.62
2012	4	0.77
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