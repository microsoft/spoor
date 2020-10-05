# Optimization ideas
* Use signed instead of unsized integer for size
  * NotPositive and NotNegative type (like not_null)
* Use index + modulus instead of iterator for circular buffer
* Do not return contiguous memory chunks from oldest to newest (just return the
  underlying data).
* Implement LIKELY and UNLIKELY macros.
* FlushQueue: Keep an atomic count for the size instead of acquiring a mutex.
