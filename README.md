# Format String Vulnerability Exploitation in an FTP Server

Welcome to this project where we explore a format string vulnerability in an FTP server. This README will provide you with insights into what a format string vulnerability is, how it can impact your code, and why it happens.
## What is a Format String Vulnerability?

In programming languages like C and C++, we often use functions like printf to work with formatted strings. However, when we allow users to supply the format string, it can introduce a dangerous bug. Let's illustrate this with an example:

Suppose we call the printf function like this: printf("%s", "Hello World!");. In this case, the output will be as expected: Hello World!.

Now, consider a scenario where we call the function like this: printf("%x");. Surprisingly, the output will be the following: 4001526c. This seemingly "random" value is, in fact, data from the stack. When calling printf without providing the expected arguments, it will unintentionally interpret and print data from the stack.
## Why Does This Happen?

This behavior occurs due to the way variadic functions are defined in C. There is no inherent mechanism within the printf function to determine the size of the VA_LIST, and the responsibility for providing the correct arguments lies with the caller. This is why printf uses the cdecl calling convention.

## The vulnerability

Lets look at the following code:
```c
void vreply(long flags, int n, char *fmt, va_list ap)
{
    char buf[BUFSIZ];
    flags &= USE_REPLY_NOTFMT | USE_REPLY_LONG;

    if (n) /* if numeric is 0, don’t output one; use n==0
              in place of printf’s */
    {
        sprintf(buf, "%03d%c", n, flags & USE_REPLY_LONG ? '-' : ' ');
        /* This is somewhat of a kludge for autospout. I personally think that
         * autospout should be done differently, but that’s not my department. -Kev
         */
        if (flags & USE_REPLY_NOTFMT)
        {
            snprintf(buf + (n ? 4 : 0), n ? sizeof(buf) - 4 : sizeof(buf), "%s", fmt);
        }
        else
        {
            vsnprintf(buf + (n ? 4 : 0), n ? sizeof(buf) - 4 : sizeof(buf), fmt, ap);
        }

        if (debug) /* debugging output :) */
        {
            syslog(LOG_DEBUG, "<--- %s", buf);
        }

        /* Yes, you want the debugging output before the client output; wrapping
         * stuff goes here, you see, and you want to log the cleartext and send
         * the wrapped text to the client.
         */
        printf("%s\r\n", buf); /* and send it to the client */

#ifdef TRANSFER_COUNT
        byte_count_total += strlen(buf);
        byte_count_out += strlen(buf);
#endif
        fflush(stdout);
    }
}
```
The vreply function accepts several parameters, including a format string (fmt) that is supplied as user input or can be influenced by user input. The format string determines how data is formatted and printed within the function.
Within the vreply function, the vsnprintf function is used to format a string and place it into a buffer (buf). This function takes the user-supplied fmt parameter as its format string. As an attacker we can supply a format string like %x to manipulate the vsnprintf function.

## The exploit
Immediately following the vulnerable vsnprintf function, there is a call to printf. This provides an opportunity for an attacker to overwrite its Global Offset Table (GOT) entry and execute their own code. In this project, we demonstrate running an exit shellcode, but theoretically, it can be any code of the attacker's choice.

## Note 
This vulnerability is well-known and serves as an educational example. It's essential to use this knowledge responsibly and ethically when addressing vulnerabilities in real-world applications.
