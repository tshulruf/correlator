#! /bin/bash
# 
# Wrap a call to wget, constructing an URL from the arguments.
# 
# $1 == s == symbol
# $2 == a == start month (January = 00)
# $3 == b == start day
# $4 == c == start year
# $5 == d == end month
# $6 == e == end day
# $7 == f == end year
# 
# Get a year's worth of data for MSFT from Sept. 22, 2010 to Sept. 22, 2011
#   sh ./wget-YF-table.sh MSFT 08 22 2010 08 22 2011
#
# 
# NOTES:
# ======
#
# http://ichart.finance.yahoo.com/table.csv?s=AA-P&a=08&b=22&c=2010&d=08&e=22&f=2011&g=d
#     s == symbol
#     a == start month
#     b == start day
#     c == start year
#     d == end month
#     e == end day
#     f == end year
#     g == d for daily (default), w for weekly, and m for monthly
#     
# Will produce "table.csv" with a header row and one value per trading date:
#     Date,Open,High,Low,Close,Volume,Adj Close
#     2011-09-22,25.30,25.65,24.60,25.06,96278300,25.06
# 
# Adj Close == Close price adjusted for dividends and splits
# 
# Will 404 on invalid symbol.
# Will give as many dates as it can as long as dates make sense. 
#     (start date in 1300AD will give all historical data till end date)
#     (end date in future will give all historical data until today)
# Invalid numbers will translate into zeros.
# 
wget -O $1.csv --random-wait -nv http://ichart.finance.yahoo.com/table.csv\?s\=$1\&a\=$2\&b\=$3\&c\=$4\&d\=$5\&e\=$6\&f\=$7\&g\=d

