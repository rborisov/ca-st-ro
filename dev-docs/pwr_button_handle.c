/*
 * http://stackoverflow.com/questions/14025794/how-can-i-detect-when-the-user-has-pressed-the-power-off-button
 */

/*
 * Class members:
 */

int m_acpidsock;
sockaddr_un m_acpidsockaddr;

/*
 * Setup code:
 */

/* Connect to acpid socket */
m_acpidsock = socket(AF_UNIX, SOCK_STREAM, 0);
if(m_acpidsock>=0)
{
    m_acpidsockaddr.sun_family = AF_UNIX;
    strcpy(m_acpidsockaddr.sun_path,"/var/run/acpid.socket");
    if(connect(m_acpidsock, (struct sockaddr *)&m_acpidsockaddr, 108)<0)
    {
        /* can't connect */
        close(m_acpidsock);
        m_acpidsock=-1;
    }
}

/*
 * Update code:
 */

/* check for any power events */
if(m_acpidsock)
{
    char buf[1024];
    int s=recv(m_acpidsock, buf, sizeof(buf), MSG_DONTWAIT);

    if(s>0)
    {
        buf[s]=0;
        printf("ACPID:%s\n\n",buf);
        if(!strncmp(buf,"button/power",12))
        {
            setShutdown();
            system("shutdown -P now");
        }
    }
}

/*
 * Close socket code:
 */

if(m_acpidsock>=0)
{
    close(m_acpidsock);
    m_acpidsock=-1;
}
