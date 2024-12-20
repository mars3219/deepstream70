template <typename TYPE>
TYPE getSum (const TYPE* pSrc, int num) {
    register TYPE sum = static_cast<TYPE> (0);
    const int chunk = 128*1024;
    while (num>chunk) {
        register TYPE partial = static_cast<TYPE> (0);
        register int n = chunk;
        while (n--) {
            partial += *pSrc++;
        }
        sum += partial;
        num -= chunk;
    }

    register TYPE partial = static_cast<TYPE> (0);
    while (num--) {
        partial += *pSr++;
    }
    sum += partial;
    return sum;
}