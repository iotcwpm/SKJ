#!/usr/bin/Rscript

# A R script to document and dump CPUE indices

# Maldives Pole and Line CPUE
#
# Uses the "Model 3: Standardized FAD" index from Table 1 of Sharma et al (2014)
# Sharma, R, Geehan, J., Adam, M.S. (2014) Maldives Skipjack Pole and Line Fishery Catch Rate Standardization 2004-2012: Reconstructing Historic CPUE till 1985. IOTC-2014-WPTT-15-XX
#
# File: CPUEseriesusedinassessmentSKPFigure3.xlsx (provided by Rishi Sharma)# 
cat(
"data_year	quarter	location	dispersion
2004	0	1.18	0.2
2004	1	1.05	0.2
2004	2	1.17	0.2
2004	3	1.44	0.2
2005	0	1.33	0.2
2005	1	1.19	0.2
2005	2	1.31	0.2
2005	3	1.63	0.2
2006	0	1.4	0.2
2006	1	1.27	0.2
2006	2	1.4	0.2
2006	3	1.73	0.2
2007	0	0.99	0.2
2007	1	0.89	0.2
2007	2	0.99	0.2
2007	3	1.22	0.2
2008	0	0.98	0.2
2008	1	0.88	0.2
2008	2	0.98	0.2
2008	3	1.23	0.2
2009	0	0.92	0.2
2009	1	0.83	0.2
2009	2	0.92	0.2
2009	3	1.12	0.2
2010	0	0.74	0.2
2010	1	0.66	0.2
2010	2	0.73	0.2
2010	3	0.91	0.2
2011	0	0.57	0.2
2011	1	0.52	0.2
2011	2	0.57	0.2
2011	3	0.7	0.2
2012	0	0.62	0.2
2012	1	0.57	0.2
2012	2	0.62	0.2
2012	3	0.77	0.2
",
file = "input/m_pl_cpue.tsv"
)

# Purse seine CPUE
#
# This is the "PSLS" index produced by Soto (2014)
# File: CPUEseriesusedinassessmentSKPFigure3.xlsx (provided by Rishi Sharma)
w_ps <- read.table(text="
data_year	location	dispersion
1984	0.53	0.3
1985	0.54	0.3
1986	0.74	0.3
1987	0.62	0.3
1988	0.91	0.3
1989	0.58	0.3
1990	0.63	0.3
1991	0.64	0.3
1992	0.65	0.3
1993	0.65	0.3
1994	0.70	0.3
1995	0.54	0.3
1996	0.53	0.3
1997	0.42	0.3
1998	0.49	0.3
1999	0.59	0.3
2000	0.82	0.3
2001	0.68	0.3
2002	1.00	0.3
2003	0.71	0.3
2004	0.68	0.3
2005	0.66	0.3
2006	0.64	0.3
2007	0.41	0.3
2008	0.47	0.3
2009	0.65	0.3
2010	0.60	0.3
2011	0.49	0.3
2012	0.38	0.3
2013	0.40	0.3
",header=T)
# Normalise to a geometric mean of 1
w_ps <- within(w_ps,{
	location <- location/exp(mean(log(location)))
})
# Write out
write.table(w_ps,file="input/w_ps_cpue.tsv",row.names=F,quote=F)
