#' obs_dist
#'
#' Plots k-values as a function of decile observation distance
#'
#' @param city nyc, washington, chicago, boston, london (case insensitive)
#' @param measure can be ('covar', 'ntrips', 'info'), where the latter is mutual
#' information.
#' @param data.dir = directory containing results from the C++ routines

obs_dist <- function (city="nyc", measure="covar", data.dir="results/")
{
    fname <- paste (data.dir, "dists/DistDeciles_", city, ".csv", sep="")
    deciles <- read.csv (fname, header=FALSE) [[1]]
    kfrom <- kto <- rep (NA, 10)
    for (i in 1:10)
    {
        dat <- fit_gaussian (city=city, measure=measure, from=TRUE,
                             nearfar=3, subscriber=i-1, data.dir=data.dir)
        kfrom [i] <- mean (dat$k, na.rm=TRUE)
        dat <- fit_gaussian (city=city, measure=measure, from=FALSE,
                             nearfar=3, subscriber=i-1, data.dir=data.dir)
        kto [i] <- mean (dat$k, na.rm=TRUE)
        cat ("\r[", i, "/10]", sep="")
    }
    cat ("\n")

    return (cbind (deciles, kto, kfrom))
}
