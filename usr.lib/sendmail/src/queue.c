# include "sendmail.h"
# include <sys/stat.h>
# include <sys/dir.h>
# include <signal.h>
# include <errno.h>

# ifndef QUEUE
SCCSID(@(#)queue.c 1.1 86/09/25 SMI (no queueing)); /* from UCB 4.5 2/15/85 */
# else QUEUE

SCCSID(@(#)queue.c 1.1 86/09/25 SMI); /* from UCB 4.5 2/15/85 */

/*
**  Work queue.
*/

struct work
{
	char		*w_name;	/* name of control file */
	long		w_pri;		/* priority of message, see below */
	struct work	*w_next;	/* next in queue */
};

typedef struct work	WORK;

WORK	*WorkQ;			/* queue of things to be done */

extern long atol();		/* C library, Ascii to long */
/*
**  QUEUEUP -- queue a message up for future transmission.
**
**	Parameters:
**		e -- the envelope to queue up.
**		queueall -- if TRUE, queue all addresses, rather than
**			just those with the QQUEUEUP flag set.
**		announce -- if TRUE, tell when you are queueing up.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The current request are saved in a control file.
*/

queueup(e, queueall, announce)
	register ENVELOPE *e;
	bool queueall;
	bool announce;
{
	char *tf;
	char *qf;
	char buf[MAXLINE];
	register FILE *tfp;
	register HDR *h;
	register ADDRESS *q;
	MAILER nullmailer;

	/*
	**  Create control file.
	*/

	tf = newstr(queuename(e, 't'));
	tfp = fopen(tf, "w");
	if (tfp == NULL)
	{
		syserr("queueup: cannot create temp file %s", tf);
		return;
	}
	(void) chmod(tf, FileMode);

# ifdef DEBUG
	if (tTd(40, 1))
		printf("queueing %s\n", e->e_id);
# endif DEBUG

	/*
	**  If there is no data file yet, create one.
	*/

	if (e->e_df == NULL)
	{
		register FILE *dfp;
		extern putbody();

		e->e_df = newstr(queuename(e, 'd'));
		dfp = fopen(e->e_df, "w");
		if (dfp == NULL)
		{
			syserr("queueup: cannot create %s", e->e_df);
			(void) fclose(tfp);
			return;
		}
		(void) chmod(e->e_df, FileMode);
		(*e->e_putbody)(dfp, ProgMailer, e);
		(void) fclose(dfp);
		e->e_putbody = putbody;
	}

	/*
	**  Output future work requests.
	**	Priority should be first, since it is read by orderq.
	*/

	/* output message priority */
	fprintf(tfp, "P%ld\n", e->e_msgpriority);

	/* output creation time */
	fprintf(tfp, "T%ld\n", e->e_ctime);

	/* output name of data file */
	fprintf(tfp, "D%s\n", e->e_df);

	/* message from envelope, if it exists */
	if (e->e_message != NULL)
		fprintf(tfp, "M%s\n", e->e_message);

	/* output name of sender */
	fprintf(tfp, "S%s\n", e->e_from.q_paddr);

	/*
	**  Output headers for this message.
	**	Expand macros completely here.  Queue run will deal with
	**	everything as absolute headers.
	**		All headers that must be relative to the recipient
	**		can be cracked later.
	**	We set up a "null mailer" -- i.e., a mailer that will have
	**	no effect on the addresses as they are output.
	*/

	bzero((char *) &nullmailer, sizeof nullmailer);
	nullmailer.m_or_rwset = nullmailer.m_os_rwset = -1;
	nullmailer.m_ir_rwset = nullmailer.m_is_rwset = -1;
	nullmailer.m_eol = "\n";
	nullmailer.m_argvsize = 10000;

	define('g', "\001f", e);
	for (h = e->e_header; h != NULL; h = h->h_link)
	{
		extern bool bitzerop();

		/* don't output null headers */
		if (h->h_value == NULL || h->h_value[0] == '\0')
			continue;

		/* don't output resent headers on non-resent messages */
		if (bitset(H_RESENT, h->h_flags) && !bitset(EF_RESENT, e->e_flags))
			continue;

		/* output this header */
		fprintf(tfp, "H");

		/* if conditional, output the set of conditions */
		if (!bitzerop(h->h_mflags) && bitset(H_CHECK|H_ACHECK, h->h_flags))
		{
			int j;

			putc('?', tfp);
			for (j = '\0'; j <= '\177'; j++)
				if (bitnset(j, h->h_mflags))
					putc(j, tfp);
			putc('?', tfp);
		}

		/* output the header: expand macros, convert addresses */
		if (bitset(H_DEFAULT, h->h_flags))
		{
			(void) expand(h->h_value, buf, &buf[sizeof buf], e);
			fprintf(tfp, "%s: %s\n", h->h_field, buf);
		}
		else if (bitset(H_FROM|H_RCPT, h->h_flags))
		{
			commaize(h, h->h_value, tfp, bitset(EF_OLDSTYLE, e->e_flags),
				 &nullmailer);
		}
		else
			fprintf(tfp, "%s: %s\n", h->h_field, h->h_value);
	}

	/* output list of recipient addresses */
	for (q = e->e_sendqueue; q != NULL; q = q->q_next)
	{
		if (queueall ? !bitset(QDONTSEND, q->q_flags) :
			       bitset(QQUEUEUP, q->q_flags))
		{
			fprintf(tfp, "R%s\n", q->q_paddr);
			if (announce)
			{
				e->e_to = q->q_paddr;
				message(Arpa_Info, "queued");
				if (LogLevel > 4)
					logdelivery("queued");
				e->e_to = NULL;
			}
#ifdef DEBUG
			if (tTd(40, 1))
			{
				printf("queueing ");
				printaddr(q, FALSE);
			}
#endif DEBUG
		}
	}

	/*
	**  Clean up.
	*/

	(void) fclose(tfp);
	qf = queuename(e, 'q');
	if (tf != NULL)
	{
		holdsigs();
		(void) unlink(qf);
		if (link(tf, qf) < 0)
			syserr("cannot link(%s, %s), df=%s", tf, qf, e->e_df);
		else
			(void) unlink(tf);
		rlsesigs();
	}

# ifdef LOG
	/* save log info */
	if (LogLevel > 15)
		syslog(LOG_DEBUG, "%s: queueup, qf=%s, df=%s\n", e->e_id, qf, e->e_df);
# endif LOG
}
/*
**  checkpoint -- rewrite the qf file to reflect the updated queue of
**	recipients.  WARNING: This function assumes that the recipient
**	lines are all at the END of the queue files, after all other lines.
**		"others" points to the list of recipients that have NOT been
**			processed yet.  We save any already processed 
**			recipients that have been marked for queuing, plus
**			all the non-processed recipients except those that
**			have already been delivered to.
**/
checkpoint(e,others)
    ENVELOPE *e;
    ADDRESS *others;
  {
    register ADDRESS *to;
    register FILE *qfp;
    char buf[MAXFIELD];
    extern char *fgetfolded();
    long position;

    if (others==NULL) return;
    qfp = fopen(queuename(e,'q'),"r+");
    if (qfp==NULL) return;

    while (fgetfolded(buf, sizeof buf, qfp) != NULL)
      {
	if (buf[0]=='R') break;
        position = ftell(qfp);
      }
    if (feof(qfp) || ferror(qfp))
      {
        /*
	 * got an error reading the queue file - give up.
	 */
	 fclose(qfp);
	 return;
      }
    fseek(qfp,position,0);
    for (to = e->e_sendqueue; to != NULL && to != others; to = to->q_next)
	if (bitset(QQUEUEUP, to->q_flags) )
	        fprintf(qfp, "R%s\n", to->q_paddr);
    if (to != NULL)
        for (; to != NULL; to = to->q_next)
	    if (!bitset(QDONTSEND, to->q_flags) || 
	         bitset(QQUEUEUP, to->q_flags) )
		    fprintf(qfp, "R%s\n", to->q_paddr);
    ftruncate(fileno(qfp), ftell(qfp));
    fclose(qfp);  
  }
/*
**  RUNQUEUE -- run the jobs in the queue.
**
**	Gets the stuff out of the queue in some presumably logical
**	order and processes them.
**
**	Parameters:
**		forkflag -- TRUE if the queue scanning should be done in
**			    a child process.  We double-fork it so it is
**			    not our child and we don't have to clean up
**			    after it.
**
**	Returns:
**		none.
**
**	Side Effects:
**		runs things in the mail queue.
*/

runqueue(forkflag)
	bool forkflag;
{
	/*
	**  See if we want to go off and do other useful work.
	*/

	if (forkflag)
	{
		int pid;

		pid = dofork();
		if (pid != 0)
		{
			/* parent -- pick up intermediate zombie */
			(void) waitfor(pid);
			if (QueueIntvl != 0)
				(void) setevent(QueueIntvl, runqueue, TRUE);
			return;
		}
		/* child -- double fork */
		if (fork() != 0)
			exit(EX_OK);
	}

	setproctitle("running queue");

# ifdef LOG
	if (LogLevel > 11)
		syslog(LOG_DEBUG, "runqueue %s, pid=%d", QueueDir, getpid());
# endif LOG

	/*
	**  Release any resources used by the daemon code.
	*/

# ifdef DAEMON
	clrdaemon();
# endif DAEMON

	/*
	**  Start making passes through the queue.
	**	First, read and sort the entire queue.
	**	Then, process the work in that order.
	**		But if you take too long, start over.
	*/

	/* order the existing work requests */
	(void) orderq(FALSE);

	/* process them once at a time */
	while (WorkQ != NULL)
	{
		WORK *w = WorkQ;

		WorkQ = WorkQ->w_next;
		dowork(w);
		free(w->w_name);
		free((char *) w);
	}
	finis();
}
/*
**  ORDERQ -- order the work queue.
**
**	Parameters:
**		listflag: TRUE if we are doing a list.
**		Otherwise, do not include requests that too "big"
**
**	Returns:
**		The number of request in the queue (not necessarily
**		the number of requests in WorkQ however).
**
**	Side Effects:
**		Sets WorkQ to the queue of available work, in order.
*/

# define WLSIZE		120	/* max size of worklist per sort */

static
orderq(listflag)
{
	register struct direct *d;
	register WORK *w;
	register WORK **wp;		/* parent of w */
	DIR *f;
	register int i;
	WORK wlist[WLSIZE+1];
	int wn = -1;
	char recipok;			/* Did recip name match? */
	extern workcmpf();
	extern int	OnlyRunId;		/* main.c */
	extern char	*OnlyRunRecip;		/* main.c */


	/* clear out old WorkQ */
	for (w = WorkQ; w != NULL; )
	{
		register WORK *nw = w->w_next;

		WorkQ = nw;
		free(w->w_name);
		free((char *) w);
		w = nw;
	}

	/* open the queue directory */
	f = opendir(".");
	if (f == NULL)
	{
		syserr("orderq: cannot open \"%s\" as \".\"", QueueDir);
		return (0);
	}

	/*
	**  Read the work directory.
	*/

	while ((d = readdir(f)) != NULL)
	{
		FILE *cf;
		char lbuf[MAXNAME];

		/* is this an interesting entry? */
		if (d->d_name[0] != 'q' || d->d_name[1] != 'f')
			continue;

		/*
		** If we're only interested in a particular job, check
		** for that one.
		*/
		if (OnlyRunId) {
			if (OnlyRunId != atoi(&d->d_name[4])) {
				continue;
			}
		}

		/* yes -- open control file (if not too many files) */
		if (++wn >= WLSIZE)
			continue;
		cf = fopen(d->d_name, "r");
		if (cf == NULL)
		{
			/* this may be some random person sending hir msgs */
			/* syserr("orderq: cannot open %s", cbuf); */
#ifdef DEBUG
			if (tTd(41, 2))
				printf("orderq: cannot open %s (%d)\n",
					d->d_name, errno);
#endif DEBUG
			errno = 0;
			wn--;
			continue;
		}
		wlist[wn].w_name = newstr(d->d_name);

		recipok = (OnlyRunRecip == 0);

		/* extract useful information */
		while (fgets(lbuf, sizeof lbuf, cf) != NULL)
		{
			if (lbuf[0] == 'P')
			{
				wlist[wn].w_pri = atol(&lbuf[1]);
				/* Stop scan unless we need to keep looking */
				if (recipok) break;
			}
			if (recipok == 0 && lbuf[0] == 'R')
			{
				register char *sp;
				for (sp = &lbuf[1]; *sp; sp++)
				{
					if (*sp == OnlyRunRecip[0] && 
					    strncmp(sp, OnlyRunRecip,
						strlen(OnlyRunRecip)) == 0) {
							recipok = 1;
							break;
					}
				}
			}
		}
		(void) fclose(cf);
		/* If recip name didn't match, don't take this queue entry */
		if ( (shouldqueue(wlist[wn].w_pri) && !listflag) ||
		       recipok == 0) wn--;
	}
	(void) closedir(f);
	wn++;

	/*
	**  Sort the work directory.
	*/

	qsort(wlist, min(wn, WLSIZE), sizeof *wlist, workcmpf);

	/*
	**  Convert the work list into canonical form.
	**	Should be turning it into a list of envelopes here perhaps.
	*/

	wp = &WorkQ;
	for (i = min(wn, WLSIZE); --i >= 0; )
	{
		w = (WORK *) xalloc(sizeof *w);
		w->w_name = wlist[i].w_name;
		w->w_pri = wlist[i].w_pri;
		w->w_next = NULL;
		*wp = w;
		wp = &w->w_next;
	}

# ifdef DEBUG
	if (tTd(40, 1))
	{
		for (w = WorkQ; w != NULL; w = w->w_next)
			printf("%32s: pri=%ld\n", w->w_name, w->w_pri);
	}
# endif DEBUG

	return (wn);
}
/*
**  WORKCMPF -- compare function for ordering work.
**
**	Parameters:
**		a -- the first argument.
**		b -- the second argument.
**
**	Returns:
**		1 if a < b
**		0 if a == b
**		-1 if a > b
**
**	Side Effects:
**		none.
*/

static
workcmpf(a, b)
	register WORK *a;
	register WORK *b;
{
	if (a->w_pri == b->w_pri)
		return (0);
	else if (a->w_pri > b->w_pri)
		return (-1);
	else
		return (1);
}
/*
**  DOWORK -- do a work request.
**
**	Parameters:
**		w -- the work request to be satisfied.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The work request is satisfied if possible.
*/

static
dowork(w)
	register WORK *w;
{
	register int i;
	ENVELOPE *newenvelope();
	extern ENVELOPE BlankEnvelope;

# ifdef DEBUG
	if (tTd(40, 1))
		printf("dowork: %s pri %ld\n", w->w_name, w->w_pri);
# endif DEBUG

	if (shouldqueue(w->w_pri))
	  {
	    if (Verbose)
		printf("\nSkipping %s\n", w->w_name);
	    return;
	  }

		/*
		**  In order to use the cache, we delay forking here.
		**
		**	Lock the control file to avoid duplicate deliveries.
		**		Then run the file as though we had just read it.
		**	We save an idea of the temporary name so we
		**		can recover on interrupt.
		*/

		/* set basic modes, etc. */
		(void) alarm(0);
		closexscript(CurEnv);
		CurEnv = newenvelope(CurEnv);
		CurEnv->e_flags = BlankEnvelope.e_flags;
		QueueRun = TRUE;
		ErrorMode = EM_MAIL;
		CurEnv->e_id = &w->w_name[2];

		setproctitle(CurEnv->e_id, 0);	/* Set process name for ps */

# ifdef LOG
		if (LogLevel > 11)
			syslog(LOG_DEBUG, "%s: dowork, pid=%d", CurEnv->e_id,
			       getpid());
# endif LOG

		/* don't use the headers from sendmail.cf... */
		CurEnv->e_header = NULL;

		/* lock the control file during processing */
		if (link(w->w_name, queuename(CurEnv, 'l')) < 0)
		{
			/* being processed by another queuer */
# ifdef LOG
			if (LogLevel > 4)
				syslog(LOG_DEBUG, "%s: locked", CurEnv->e_id);
# endif LOG
			return;
		}

		/* do basic system initialization */
		initsys();

		/* read the queue control file */
		readqf(CurEnv, TRUE);
		CurEnv->e_flags |= EF_INQUEUE;
		eatheader(CurEnv);

		/* do the delivery */
		if (!bitset(EF_FATALERRS, CurEnv->e_flags))
			sendall(CurEnv, SM_DELIVER);

	dropenvelope(CurEnv);
}
/*
**  READQF -- read queue file and set up environment.
**
**	Parameters:
**		e -- the envelope of the job to run.
**		full -- if set, read in all information.  Otherwise just
**			read in info needed for a queue print.
**
**	Returns:
**		none.
**
**	Side Effects:
**		cf is read and created as the current job, as though
**		we had been invoked by argument.
*/

static
readqf(e, full)
	register ENVELOPE *e;
	bool full;
{
	char *qf;
	register FILE *qfp;
	char buf[MAXFIELD];
	extern char *fgetfolded();

	/*
	**  Read and process the file.
	*/

	qf = queuename(e, 'q');
	qfp = fopen(qf, "r");
	if (qfp == NULL)
	{
		syserr("readqf: no control file %s", qf);
		return;
	}
	FileName = qf;
	LineNumber = 0;
	if (Verbose && full)
		printf("\nRunning %s\n", e->e_id);
	while (fgetfolded(buf, sizeof buf, qfp) != NULL)
	{
		switch (buf[0])
		{
		  case 'R':		/* specify recipient */
			sendtolist(&buf[1], (ADDRESS *) NULL, &e->e_sendqueue);
			break;

		  case 'H':		/* header */
			if (full)
				(void) chompheader(&buf[1], FALSE);
			break;

		  case 'M':		/* message */
			e->e_message = newstr(&buf[1]);
			break;

		  case 'S':		/* sender */
			setsender(newstr(&buf[1]));
			break;

		  case 'D':		/* data file name */
			if (!full)
				break;
			e->e_df = newstr(&buf[1]);
			e->e_dfp = fopen(e->e_df, "r");
			if (e->e_dfp == NULL)
				syserr("readqf: cannot open %s", e->e_df);
			break;

		  case 'T':		/* init time */
			e->e_ctime = atol(&buf[1]);
			break;

		  case 'P':		/* message priority */
			e->e_msgpriority = atol(&buf[1]);

			/* make sure that big things get sent eventually */
			e->e_msgpriority -= WKTIMEFACT;
			break;

		  case '\0':		/* blank line, just ignore */
			break;

		  default:
			syserr("%s: bad line %d: \"%s\"", e->e_id, LineNumber, 
				buf);
			break;
		}
	}
	fclose(qfp);

	/*
	 * Following test inserted by JCGilmore at Sun, 25 Feb 83.
	 * Sendmail crashes often leave around empty queue files.
	 * On Vaxen, things work out such that the file is deleted next
	 * time it is processed.
	 *
	 * On 68000's, reading such a file crashes the mailer because
	 * it leaves various pointers set to zero in the envelope.
	 * We really should check for all the relevant pointers being
	 * set, but since I don't know which are relevant, I'll just
	 * check for empty file here.
	 *
	 * The flags we insert cause the message to be removed from
	 * the queue, sending to be avoided, and error response mailing
	 * to also be avoided (since we mark it as already being an error
	 * response -- it sorta is; leaving empty files around is
	 * sendmail's response to certain kinds of errors...).
	 */
	if (LineNumber == 0) {
		errno = 0;
		e->e_flags |= EF_CLRQUEUE	/* Remove from queue */
			   |  EF_FATALERRS	/* Msg has died */
			   |  EF_RESPONSE;	/* Msg is an error response */
	}

	FileName = NULL;
}
/*
**  PRINTQUEUE -- print out a representation of the mail queue
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Prints a listing of the mail queue on the standard output.
*/

printqueue()
{
	register WORK *w;
	FILE *f;
	int nrequests;
	char buf[MAXLINE];

	/*
	**  Read and order the queue.
	*/

	nrequests = orderq(TRUE);

	/*
	**  Print the work list that we have read.
	*/

	/* first see if there is anything */
	if (nrequests <= 0)
	{
		printf("Mail queue is empty\n");
		return;
	}

	printf("\t\tMail Queue (%d request%s", nrequests, nrequests == 1 ? "" : "s");
	if (nrequests > WLSIZE)
		printf(", only %d printed", WLSIZE);
	printf(")\n--QID-- --Size-- -----Q-Time----- ------------Sender/Recipient------------\n");
	for (w = WorkQ; w != NULL; w = w->w_next)
	{
		struct stat st;
		auto time_t submittime = 0;
		long dfsize = -1;
		char lf[20];
		char message[MAXLINE];

		printf("%7s", w->w_name + 2);
		strcpy(lf, w->w_name);
		lf[0] = 'l';
		if (stat(lf, &st) >= 0)
			printf("*");
		else
			printf(" ");
		if (shouldqueue(w->w_pri))
			printf("X");
		else
			printf(" ");
		errno = 0;
		f = fopen(w->w_name, "r");
		if (f == NULL)
		{
			printf(" (finished)\n");
			errno = 0;
			continue;
		}
		message[0] = '\0';
		while (fgets(buf, sizeof buf, f) != NULL)
		{
			fixcrlf(buf, TRUE);
			switch (buf[0])
			{
			  case 'M':	/* error message */
			  	strcpy(message,"(");
				strcat(message, &buf[1]);
				strcat(message,")");
				break;

			  case 'S':	/* sender name */
				printf("%7ld %.16s %.45s", dfsize,
					ctime(&submittime), &buf[1]);
				if (message[0] != '\0')
					printf("\n%57s", message);
				break;

			  case 'R':	/* recipient name */
				printf("\n\t\t\t\t  %.45s", &buf[1]);
				break;

			  case 'T':	/* creation time */
				submittime = atol(&buf[1]);
				break;

			  case 'D':	/* data file name */
				if (stat(&buf[1], &st) >= 0)
					dfsize = st.st_size;
				break;
			}
		}
		if (submittime == (time_t) 0)
			printf(" (no control file)");
		printf("\n");
		fclose(f);
	}
}

# endif QUEUE
/*
**  QUEUENAME -- build a file name in the queue directory for this envelope.
**
**	Assigns an id code if one does not already exist.
**	This code is very careful to avoid trashing existing files
**	under any circumstances.
**		We first create an nf file that is only used when
**		assigning an id.  This file is always empty, so that
**		we can never accidently truncate an lf file.
**
**	Parameters:
**		e -- envelope to build it in/from.
**		type -- the file type, used as the first character
**			of the file name.
**
**	Returns:
**		a pointer to the new file name (in a static buffer).
**
**	Side Effects:
**		Will create the lf and qf files if no id code is
**		already assigned.  This will cause the envelope
**		to be modified.
*/

char *
queuename(e, type)
	register ENVELOPE *e;
	char type;
{
	static char buf[MAXNAME];
	static int pid = -1;
	char c1 = 'A';
	char c2 = 'A';

	if (e->e_id == NULL)
	{
		char qf[20];
		char nf[20];
		char lf[20];

		/* find a unique id */
		if (pid != getpid())
		{
			/* new process -- start back at "AA" */
			pid = getpid();
			c1 = 'A';
			c2 = 'A' - 1;
		}
		(void) sprintf(qf, "qfAA%05d", pid);
		strcpy(lf, qf);
		lf[0] = 'l';
		strcpy(nf, qf);
		nf[0] = 'n';

		while (c1 < '~' || c2 < 'Z')
		{
			int i;

			if (c2 >= 'Z')
			{
				c1++;
				c2 = 'A' - 1;
			}
			lf[2] = nf[2] = qf[2] = c1;
			lf[3] = nf[3] = qf[3] = ++c2;
# ifdef DEBUG
			if (tTd(7, 20))
				printf("queuename: trying \"%s\"\n", nf);
# endif DEBUG
# ifdef QUEUE
			if (access(lf, 0) >= 0 || access(qf, 0) >= 0)
				continue;
			errno = 0;
			i = creat(nf, FileMode);
			if (i < 0)
			{
				(void) unlink(nf);	/* kernel bug */
				continue;
			}
			(void) close(i);
			i = link(nf, lf);
			(void) unlink(nf);
			if (i < 0)
				continue;
			if (link(lf, qf) >= 0)
				break;
			(void) unlink(lf);
# else QUEUE
			if (close(creat(qf, FileMode)) >= 0)
				break;
# endif QUEUE
		}
		/*
		 * The next test used to check for c1 >= '~', but there's
		 * no reason to go thru 26**2 permutations; 52 should do.
		 */
		if (c1 >= 'B' && c2 >= 'Z')
		{
			if (type != '\0') {
			     syserr("queuename: Cannot create \"%s\" in \"%s\"",
				    qf, QueueDir);
			}
			abort();
		}
		e->e_id = newstr(&qf[2]);
		define('i', e->e_id, e);
# ifdef DEBUG
		if (tTd(7, 1))
			printf("queuename: assigned id %s, env=%x\n", e->e_id, e);
# ifdef LOG
		if (LogLevel > 16)
			syslog(LOG_DEBUG, "%s: assigned id", e->e_id);
# endif LOG
# endif DEBUG
	}

	if (type == '\0')
		return (NULL);
	(void) sprintf(buf, "%cf%s", type, e->e_id);
# ifdef DEBUG
	if (tTd(7, 2))
		printf("queuename: %s\n", buf);
# endif DEBUG
	return (buf);
}
/*
**  UNLOCKQUEUE -- unlock the queue entry for a specified envelope
**
**	Parameters:
**		e -- the envelope to unlock.
**
**	Returns:
**		none
**
**	Side Effects:
**		unlocks the queue for `e'.
*/

unlockqueue(e)
	ENVELOPE *e;
{
	if (e->e_id == NULL) return;
	/* remove the transcript */
#ifdef DEBUG
# ifdef LOG
	if (LogLevel > 19)
		syslog(LOG_DEBUG, "%s: unlock", e->e_id);
# endif LOG
	if (!tTd(51, 4))
#endif DEBUG
		xunlink(queuename(e, 'x'));

# ifdef QUEUE
	/* last but not least, remove the lock */
	xunlink(queuename(e, 'l'));
# endif QUEUE
}
