#' get_bike_data
#'
#' Loads hire bicycle data pre-generated by the C++ routines in
#' github.com/mpadge/bike-correlations. 
#'
#' @param city nyc, washington, chicago, boston, london (case insensitive)
#' @param from TRUE or FALSE
#' @param measure can be ('covar', 'ntrips', 'info'), where the latter is mutual
#' information.
#' @param std is not implemented
#' @param nearfar = (0,1,2) for (all, near, far)
#' @param subscriber = 0 to analyse all data; 1 to analyse subscribers only; 2
#' to analyse non-subscribers only; 3 to perform aged-based analyses
#' @param mf = 1/2 for male/female data (for subscriber=1 only)
#' @param data.dir = directory containing results from the C++ routines
#' @return a list of 2 matrices of d=inter-station distances, y=value according
#' to measure parameter, along with xy=lat-lon coordinates of stations.

get_bike_data <- function (city="nyc", from=TRUE, measure='covar', std=TRUE, 
                      nearfar=0, subscriber=0, mf=0, data.dir="../../results/",
                      msg=FALSE)
{
    txt.dir <- data.dir
    if (subscriber > 2)
        txt.dir <- paste (txt.dir, "age/", sep="")
    fname <- paste (txt.dir, "stationDistsMat-", city, ".csv", sep="")
    dists <- as.matrix (read.csv (fname, header=FALSE))
    dims <- dim (dists)
    dists <- array (dists, dim=dims)

    if (from) txt.ft <- "from"
    else txt.ft <- "to"
    if (nearfar == 0) txt.nf <- "all"
    else if (nearfar == 1) txt.nf <- "near"
    else txt.nf <- "far"
    if (subscriber == 0 | subscriber == 2)
        mf = 0

    if (tolower (measure) == 'covar' | tolower (measure) == "info" |
        tolower (measure) == "mi")
    {
        if (measure == "covar") 
            prefix <- "Cov"
        else
            prefix <- "MI"
        if (std) txt.sd <- "std_"
        else txt.sd <- "unstd_" # these files don't currently exist
        if (tolower (measure) == "info" | tolower (measure) == "mi")
            txt.sd <- ""
        fname <- paste (txt.dir, prefix, "_", city, "_", txt.ft, 
                        "_", txt.sd, txt.nf, sep="")
        indx <- which (dists < 0) # the latter is for -DOUBLE_MAX
        if (city == "nyc" | city == "boston" | city == "chicago")
            fname <- paste (fname, "_", subscriber, mf, sep="")
    } else {
        fname <- paste (txt.dir, "NumTrips_", city, sep="")
        if (city == "nyc" | city == "boston" | city == "chicago")
            fname <- paste (fname, "_", subscriber, mf, sep="")
    }
    fname <- paste (fname, ".csv", sep="")
    cat ("***", fname, "\n")
    y <- as.matrix (read.csv (fname, header=FALSE))
    y <- array (y, dim=dims)

    if (measure != 'ntrips')
        indx <- which (dists < 0 | y == min (y, na.rm=TRUE)) 
    else
        indx <- which (dists < 0)
    dists [indx] <- NA
    y [indx] <- NA
    if (tolower (measure) == "info" | tolower (measure) == "mi")
        y <- -y

    # Also load station coordinates
    wd <- getwd ()
    setwd (data.dir)
    setwd ("../data/")
    if (city == "boston")
    {
        fname <- "hubway_stations.csv"
        cols <- c ("lat", "lng")
    }
    else if (city == "chicago")
    {
        fname <- "Divvy_Stations_2014-Q3Q4.csv"
        cols <- c ("latitude", "longitude")
    }
    else
    {
        fname <- paste ("station_latlons_", city, ".txt", sep="")
        cols <- c ("lat", "long")
    }
    stns <- read.csv (fname, header=TRUE)
    xy <- cbind (stns [,cols])
    names (xy) <- c ("lat", "lon")
    setwd (wd)

    return (list (d=dists, y=y, xy=xy))
} 