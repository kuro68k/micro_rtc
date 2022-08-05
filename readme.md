# µRTC - an 8/32 bit RTC for embedded systems

µRTC (Micro RTC) is an RTC for embedded systems with the following features:

* 8 or 32 bit native
* Seconds Since Epoch (SSE) or Split (y/m/d h/m/s) modes
* 1 second resolution
* `2000/01/01 00:00:00 UTC` epoch
* EU DST calculation
* UTC to local time calculation
* Time handling utility functions
* Build time and date utility functions
* GPLv3 licence

## Seconds Since Epoch Mode or Split Mode
Seconds Since Epoch (SSE) mode tracks the number of seconds since the epoch time. Split mode uses a Gregorian calendar that tracks year, month, day, hour, minute and second separately.

The choice of which to use depends on your needs. Split mode is useful for things like clocks, where displaying time in human readable format is the main purpose. SSE mode makes handling time easier, because time is simply a 32 bit unsigned number that can have arithmetic performed on it, and be trivially compared to other times.

SSE mode also has a simpler tick function (see below).

## 8 or 32 bit mode
Select 8 or 32 bit mode to match the architecture of your system, or according to your needs. In Split Mode, the year, month, day, hour, minute and second are all stored as 8 or 32 bits accordingly. In 8 bit mode the year is stored as the number of years since the epoch, i.e. year 0 is 2000. In 32 bit mode, the year is stored in full (e.g. 2022), but cannot be lower than the epoch (2000) or some functions will not work correctly.

| Mode | Range |
|-|-
| 8 bit SSE | `2000/01/01 00:00:00 UTC` to `2136/02/07 06:28:15 UTC`.
| 8 bit Split | `2000/01/01 00:00:00 UTC` to `2255/12/31 23:59:59 UTC`<sup>1</sup>.
| 32 bit SSE | `2000/01/01 00:00:00 UTC` to `2136/02/07 06:28:15 UTC`
| 32 bit Split | `2000/01/01 00:00:00 UTC` to `2400/01/01 00:00:00 UTC`<sup>2</sup>. 

<sup>1</sup> While times beyond `2136/02/07 06:28:15 UTC` can be stored as split times, they cannot be converted to SSE times due to the limits of 32 bit unsigned numbers.

<sup>2</sup> As (<sup>1</sup>), but additionally in 32 bit mode dates beyond the year 2400 are possible but not tested. See the Other Limitations section below for other important information about very high year numbers.

A mismatch between the processor architecture and the RTC will result in lower performance, but for most applications it won't make much difference. 8 bit uses less memory on 8 bit machines too.

## Div/mul Mode or Iterative Mode

When converting between SSE and split mode, as well as certain other calculations, there are two ways to do them. The first is using division and multiplication. That is the recommended mode in most cases, as it can take advantage of any available hardware for those operations, and because even when done in software it results in a predictable maximum execution time. It also re-uses code for software implementations of multiplication and division.

The other option is to use iteration. The upside is that it avoids potentially expensive operations on platforms where multiplication and division are very slow. The downside is that execution time varies considerably depending on the date.

## Tick
A tick function, `RTC_tick()`, is called once per second to advance the RTC. In SSE mode all it does is add 1 to the 32 bit time value, so its execution time is fixed and fairly short. That makes it ideal for calling from an interrupt.

In Split mode the execution time is longer, and will vary as sometimes only the second counter is incremented, other times the year, month, day, hour, minute and second counters all need to be incremented.

## EU DST calculation
The EU has settled on a simple way to calculate the start and end dates of DST, and stuck with it. As of 2022, the calculation is accurate, but may need to be updated in future. The EU has been talking about abandoning DST for some time, so it is recommended to have some way of switching DST off in your system. If you are concerned about long term accuracy, while it is unlikely that the EU will change the data calculation, the best option is to store the dates somewhere like EEPROM so that they can be updated later.

## Build time/date

`build.c` contains code to create strings that can be used to timestamp each build. To use them, make sure that your build process touches `build.c` so that the time and date are updated. The strings are built by the preprocessor, so have no runtime overhead.

`build_timestamp` contains the built timestamp in human readable format, basically ISO 8601 but with a space instead of the T separate date and time.

`build_number` is similar but contains only digits.

Be aware that these strings use the `__DATE__` and `__TIME__` macros, which are usually local time. If you want `build_number` to be unique you will need to account for DST changeover times, e.g. by avoiding building at that time or by setting the machine to use UTC. A similar issue can happen if the build machine's clock is changed, although with periodic NTP sync it rarely gets more than a few seconds out.
