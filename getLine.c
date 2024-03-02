#include "shell.h"

/**
 * input_buf - buffers chained commands
 * @info: parameter struct
 * @buf: address of buffer
 * @len: address of len var
 *
 * Return: bytes read
 */
ssize_t input_buf(info_t *info, char **buf, size_t *len)
{
	ssize_t d = 0;
	size_t len_p = 0;

	if (!*len) /* if nothing left in the buffer, fill it */
	{
		/*bfree((void **)info->cmd_buf);*/
		free(*buf);
		*buf = NULL;
		signal(SIGINT, sigintHandler);
#if USE_GETLINE
		d = getline(buf, &len_p, stdin);
#else
		d = _getline(info, buf, &len_p);
#endif
		if (d > 0)
		{
			if ((*buf)[d - 1] == '\n')
			{
				(*buf)[d - 1] = '\0'; /* remove trailing newline */
				d--;
			}
			info->linecount_flag = 1;
			remove_comments(*buf);
			build_history_list(info, *buf, info->histcount++);
			/* if (_strchr(*buf, ';')) is this a command chain? */
			{
				*len = d;
				info->cmd_buf = buf;
			}
		}
	}
	return (d);
}

/**
 * get_input - gets a line minus the newline
 * @info: parameter struct
 *
 * Return: bytes read
 */
ssize_t get_input(info_t *info)
{
	static char *buf; /* the ';' command chain buffer */
	static size_t b, c, len;
	ssize_t d = 0;
	char **buf_p = &(info->arg), *p;

	_putchar(BUF_FLUSH);
	d = input_buf(info, &buf, &len);
	if (d == -1) /* EOF */
		return (-1);
	if (len)	/* we have commands left in the chain buffer */
	{
		c = b; /* init new iterator to current buf position */
		p = buf + b; /* get pointer for return */

		check_chain(info, buf, &c, b, len);
		while (c < len) /* iterate to semicolon or end */
		{
			if (is_chain(info, buf, &c))
				break;
			c++;
		}

		b = c + 1; /* increment past nulled ';'' */
		if (b >= len) /* reached end of buffer? */
		{
			b = len = 0; /* reset position and length */
			info->cmd_buf_type = CMD_NORM;
		}

		*buf_p = p; /* pass back pointer to current command position */
		return (_strlen(p)); /* return length of current command */
	}

	*buf_p = buf; /* else not a chain, pass back buffer from _getline() */
	return (d); /* return length of buffer from _getline() */
}

/**
 * read_buf - reads a buffer
 * @info: parameter struct
 * @buf: buffer
 * @i: size
 *
 * Return: r
 */
ssize_t read_buf(info_t *info, char *buf, size_t *b)
{
	ssize_t d = 0;

	if (*b)
		return (0);
	d = read(info->readfd, buf, READ_BUF_SIZE);
	if (d >= 0)
		*b = d;
	return (d);
}

/**
 * _getline - gets the next line of input from STDIN
 * @info: parameter struct
 * @ptr: address of pointer to buffer, preallocated or NULL
 * @length: size of preallocated ptr buffer if not NULL
 *
 * Return: s
 */
int _getline(info_t *info, char **ptr, size_t *length)
{
	static char buf[READ_BUF_SIZE];
	static size_t b, len;
	size_t k;
	ssize_t d = 0, s = 0;
	char *p = NULL, *new_p = NULL, *a;

	p = *ptr;
	if (p && length)
		s = *length;
	if (b == len)
		b = len = 0;

	d = read_buf(info, buf, &len);
	if (d == -1 || (d == 0 && len == 0))
		return (-1);

	a = _strchr(buf + b, '\n');
	k = a ? 1 + (unsigned int)(a - buf) : len;
	new_p = _realloc(p, s, s ? s + k : k + 1);
	if (!new_p) /* MALLOC FAILURE! */
		return (p ? free(p), -1 : -1);

	if (s)
		_strncat(new_p, buf + b, k - b);
	else
		_strncpy(new_p, buf + b, k - b + 1);

	s += k - b;
	b = k;
	p = new_p;

	if (length)
		*length = s;
	*ptr = p;
	return (s);
}

/**
 * sigintHandler - blocks ctrl-C
 * @sig_num: the signal number
 *
 * Return: void
 */
void sigintHandler(__attribute__((unused))int sig_num)
{
	_puts("\n");
	_puts("$ ");
	_putchar(BUF_FLUSH);
}
