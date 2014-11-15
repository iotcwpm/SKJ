#!/usr/bin/Rscript

# A R script to document and dump CPUE indices

# Maldives Pole and Line CPUE
#
# Uses the "Model 3: Standardized FAD" index from Table 1 of Sharma et al (2014)
# Sharma, R, Geehan, J., Adam, M.S. (2014) Maldives Skipjack Pole and Line Fishery Catch Rate Standardization 2004-2012: Reconstructing Historic CPUE till 1985. IOTC-2014-WPTT-15-XX
#
# File: CPUEseriesusedinassessmentSKPFigure3.xlsx (provided by Rishi Sharma)# 
cat(
"year	quarter	index
2004	0	1.18
2004	1	1.05
2004	2	1.17
2004	3	1.44
2005	0	1.33
2005	1	1.19
2005	2	1.31
2005	3	1.63
2006	0	1.4
2006	1	1.27
2006	2	1.4
2006	3	1.73
2007	0	0.99
2007	1	0.89
2007	2	0.99
2007	3	1.22
2008	0	0.98
2008	1	0.88
2008	2	0.98
2008	3	1.23
2009	0	0.92
2009	1	0.83
2009	2	0.92
2009	3	1.12
2010	0	0.74
2010	1	0.66
2010	2	0.73
2010	3	0.91
2011	0	0.57
2011	1	0.52
2011	2	0.57
2011	3	0.7
2012	0	0.62
2012	1	0.57
2012	2	0.62
2012	3	0.77
",
file = "processed-data/m-pl-cpue.tsv"
)

# Purse seine CPUE
#
# This is the "PSLS" index produced by Soto (2014)
# File: CPUEseriesusedinassessmentSKPFigure3.xlsx (provided by Rishi Sharma)
cat(
"year	index
1984	0.53
1985	0.54
1986	0.74
1987	0.62
1988	0.91
1989	0.58
1990	0.63
1991	0.64
1992	0.65
1993	0.65
1994	0.70
1995	0.54
1996	0.53
1997	0.42
1998	0.49
1999	0.59
2000	0.82
2001	0.68
2002	1.00
2003	0.71
2004	0.68
2005	0.66
2006	0.64
2007	0.41
2008	0.47
2009	0.65
2010	0.60
2011	0.49
2012	0.38
2013	0.40
",
file = "processed-data/w-ps-cpue.tsv"
)