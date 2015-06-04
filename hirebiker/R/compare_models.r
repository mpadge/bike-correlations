#' compare_models
#'
#' Statistical comparisons of 6 different functional forms of distance decay:
#' power-law, Gaussian, exponential, Weibull, Cauchy, and Box-Cox
#'
#' @param city nyc, washington, chicago, boston, london (case insensitive)
#' @param from TRUE or FALSE
#' @param measure can be ('covar', 'ntrips', 'info'), where the latter is mutual
#' information.
#' @param plot = TRUE allows visual inspection of individual station results
#' @return a list of data frames, one for each model, containg data frame with
#' rows for each station and columns of [SS, k, b, AIC], where k is the width
#' parameter of the model, and b is the shape parameter (which does not apply to
#' power-law, Gaussian or Cauchy models).

compare_models <- function (city="nyc", from=TRUE, measure='covar', plot=FALSE,
                           data.dir="./results/")
{
    if (measure == 'covar')
        cat (city, ": Fitting distance decays to covariances\n")
    else if (measure == "info")
        cat (city, ": Fitting distance decays to mutual information\n")
    else
        cat (city, ": Fitting distance decays to absolute numbers of trips\n")

    dat <- get_bike_data (city=city, from=from, measure=measure, std=TRUE, 
                    nearfar=0, subscriber=0, mf=0, msg=TRUE,
                    data.dir=data.dir)

    n <- dim (dat$d)[1]

    if (measure == 'covar')
        ssmult <- 1e12
    else
        ssmult <- 1 / 10000

    # ***** THE FUNCTIONAL FORMS *****
    # For all models, b and k are respective form and width parameters.
    # Covariances generally decay to a slightly negative value, requiring all
    # models to include an additive y0 parameter. This is also necessary for MI,
    # which increases from zero, and so models are fitted to negative values,
    # ultimately decaying to some value << 0.
    mods <- list ()
    mods [[1]] <- function (y, d, a0=2*mean(y), k0, b0) # Gaussian (b0 not used)
                tryCatch (nls (y ~ y0 + a * exp (-(d / k)^2), 
                            start = list (a = 2*mean(y), k = k0, y0=0)),
                            error=function (e) NULL)
    mods [[2]] <- function (y, d, a0=2*mean(y), k0, b0) # Exponential
                tryCatch (nls (y ~ y0 + a * exp (-(d / k) ^ b), 
                            start=list(a=a0,k=k0, b=2, y0=0)),
                            error=function (e) NULL)
    mods [[3]] <- function (y, d, a0=2*mean(y), k0, b0) # Weibull
                tryCatch (nls (y ~ y0 + a * (b / k) * (d / k) ^ (b - 1) * 
                            exp (-(d / k) ^ b),
                            start=list(a=a0, k=k0, b=b0, y0=0)),
                            error=function (e) NULL)
    mods [[4]] <- function (y, d, a0=2*mean(y), k0, b0) # Cauchy (b0 not used)
                tryCatch (nls (y ~ y0 + (a / k) * (k ^ 2 / (d ^ 2 + k ^ 2)),
                            start=list(a=a0, k=k0, y0=0)),
                            error=function (e) NULL)
    mods [[5]] <- function (y, d, a0=2*mean(y), k0, b0) # Box-Cox
                tryCatch (nls (y ~ y0 + a * exp (-d^(2*b) / (k^2*b^2)), 
                            start = list (a = a0, k = k0, b=b0, y0=0)),
                            error=function (e) NULL)
    # ***** END FUNCTIONAL FORMS *****

    mod1 <- data.frame (array (NA, dim=c(n, 3)))
    names (mod1) <- c ("ss", "k", "aic")
    mod2 <- data.frame (array (NA, dim=c(n, 4)))
    names (mod2) <- c ("ss", "k", "b", "aic")
    results <- list (pow=mod1, gauss=mod1, expo=mod2, weib=mod2, cauch=mod1,
                   boxcox=mod2)
    cols <- c ("black", "red", "red", "blue", "lawngreen", "magenta")
    ltys <- c (1, 1, 2, 1, 1, 1)

    for (i in 1:n)
    {
        d <- dat$d [i, ]
        y <- dat$y [i, ]
        indx <- which (!is.na (d) & !is.na (y))
        # There are some cases for which trip data exist yet station distances
        # haven't yet been added to list. These produce NAs in the dists table.
        # The sd(y) condition prevents analyses of !covar for which all trips,
        # and thus all y-values, are zero.
        if (length (indx) > 10 & sd (y, na.rm=TRUE) > 0)
        {
            d <- d[indx]
            y <- y[indx]
            if (plot)
            {
                plot (d, y, pch=1, col="orange")
                lines (range (d), c (0, 0), col="gray", lty=2)
            }
            dfit <- seq(min(d), max(d), length.out=100)

            # ***** power-law is done separately:
            if (tolower (measure) == "info" | tolower (measure) == "mi")
            {
                indx <- which (d > 0) # Because 0 distances do occur
                y2 <- -y[indx]
            }
            else
            {
                indx <- which (y > 0 & d > 0) # Because 0 distances do occur
                y2 <- y [indx]
            }
            d2 <- d [indx]
            mod <- lm (log10 (y2) ~ log10 (d2))
            coeffs <- summary (mod)$coefficients
            yfit <- 10 ^ (coeffs [1] + log10 (d2) * coeffs [2])
            results$pow$ss [i] <- mean ((yfit - y2) ^ 2) * ssmult
            results$pow$k [i] <- coeffs [2]
            results$pow$aic [i] <- AIC (mod)
            if (plot)
            {
                yfit <- 10 ^ (coeffs [1] + log10 (dfit) * coeffs [2])
                if (tolower (measure) == "info" | tolower (measure) == "mi")
                    yfit <- -yfit
                lines (dfit, yfit, col=cols [1], lty=ltys [1])
            }

            # Then the remaining 5 models
            for (j in 1:length (mods))
            {
                k0 <- 0
                b0 <- 1
                mod <- NULL
                while (is.null (mod) & k0 < 10)
                {
                    k0 <- k0 + 1
                    mod <- do.call (mods [[j]], 
                                    list(y=y, d=d, a0=2*mean (y), k0, b0))
                }
                if (!is.null (mod))
                {
                    yfit <- mean ((predict (mod) - y) ^ 2)
                    results [[j + 1]]$ss [i] <- yfit * ssmult
                    results [[j + 1]]$k [i] <- summary (mod)$coefficients [2]
                    results [[j + 1]]$aic [i] <- AIC (mod)
                    if (dim (results[[j+1]])[2] > 3)
                        results [[j + 1]]$b <- summary (mod)$coefficients [3]
                    if (plot)
                    {
                        yfit <- predict (mod, new=data.frame (d=dfit))
                        lines (dfit, yfit, col=cols [j+1], lty=ltys [j+1],
                               lwd=2)
                    }
                }
            }
            if (plot)
            {
                xpos <- min (dfit) + 0.75 * diff (range (dfit))
                legend (xpos, max (y), lwd=2, col=cols, lty=ltys,
                        bty="n", legend=c("power","gauss","exp","weibull",
                                          "cauchy","boxcox"))
                title (main = i)
                loc <- locator (n=1)
            }
            else
                cat ("\r", i, "/", n, sep="")
        } # end if len (indx) > 10
    } # end for i
    cat ("\n")

    if (tolower (measure) == "info" | tolower (measure) == "mi")
        results$pow$k <- -results$pow$k

    # Box-Cox fits yield occassionally enormous k-values, so
    indx <- which (results$boxcox$k > (5 * median (results$boxcox$k, na.rm=T)))
    results$boxcox [indx,] <- NA
    # And sign of Gaussian k is sometimes negative, so
    results$gauss$k <- abs (results$gauss$k)

    cat ("Model\t|\tSS\tAIC\t|\tk\t\tb\t|\n")
    cat (c (rep ("-", 66), "\n"), sep="")
    for (i in 1:length (results))
    {
        ss <- formatC (mean (results [[i]]$ss, na.rm=TRUE),
                    format="f", digits=1)
        aic <- round (mean (results [[i]]$aic, na.rm=TRUE))
        k <- formatC (mean (results [[i]]$k, na.rm=TRUE),
                    format="f", digits=2)
        if (dim (results [[i]])[2] > 3)
            b <- formatC (mean (results [[i]]$b, na.rm=TRUE),
                    format="f", digits=2)
        else
            b <- ""
        cat (names (results) [i], "\t|\t", ss, "\t", aic, "\t|\t",
             k, "\t\t")
        if (dim (results [[i]])[2] == 3)
            cat ("\t|\n")
        else
            cat (b, "\t|\n")

    }
    cat (c (rep ("-", 66), "\n"), sep="")

    return (results)
} # end compare.models()
#cities <- c ("nyc", "washingtondc", "chicago", "boston", "london")
#for (i in 1:length (cities))
#    dat <- compareModels (city = cities [i])
