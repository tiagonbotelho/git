#include "cache.h"
#include "commit.h"
#include "config.h"
#include "revision.h"
#include "argv-array.h"
#include "list-objects.h"
#include "list-objects-filter.h"
#include "list-objects-filter-options.h"

/*
 * Reject the arg if it contains any characters that might
 * require quoting or escaping when handing to a sub-command.
 */
static int reject_injection_chars(const char *arg)
{
	const unsigned char *p;

	for (p = (const unsigned char *)arg; *p; p++) {
		if (*p < 0x20) /* control character */
			return 1;
		if (*p >= '0' && *p <= '9')
			continue;
		if (*p >= 'A' && *p <= 'Z')
			continue;
		if (*p >= 'a' && *p <= 'z')
			continue;
		if (*p >= 0x80)
			continue;

		switch (*p) {
		case ' ': return 1; /* 0x20 */
		case '!': continue; /* 0x21 */
		case '"': return 1; /* 0x22 */
		case '#': return 1; /* 0x23 */
		case '$': return 1; /* 0x24 */
		case '%': continue; /* 0x25 */
		case '&': return 1; /* 0x26 */
		case '\'':return 1; /* 0x27 */
		case '(': continue; /* 0x28 */
		case ')': continue; /* 0x29 */
		case '*': return 1; /* 0x2a */
		case '+': return 1; /* 0x2b */
		case ',': continue; /* 0x2c */
		case '-': continue; /* 0x2d */
		case '.': continue; /* 0x2e */
		case '/': continue; /* 0x2f */

		case ':': continue; /* 0x3a */
		case ';': return 1; /* 0x3b */
		case '<': return 1; /* 0x3c */
		case '=': continue; /* 0x3d */
		case '>': return 1; /* 0x3e */
		case '?': continue; /* 0x3f */

		case '@': continue; /* 0x40 */

		case '[': continue; /* 0x5b */
		case '\\':return 1; /* 0x5c */
		case ']': continue; /* 0x5d */
		case '^': continue; /* 0x5e */
		case '_': continue; /* 0x5f */

		case '`': return 1; /* 0x60 */

		case '{': continue; /* 0x7b */
		case '|': return 1; /* 0x7c */
		case '}': continue; /* 0x7d */
		case '~': continue; /* 0x7e */
		case 0x7f:return 1; /* 0x7f */
		default:  continue;
		}
	}
	return 0;
}

/*
 * Parse value of the argument to the "filter" keword.
 * On the command line this looks like:
 *       --filter=<arg>
 * and in the pack protocol as:
 *       "filter" SP <arg>
 *
 * <arg> ::= blob:none
 *           blob:limit=<n>[kmg]
 *           sparse:oid=<oid-expression>
 *           sparse:path=<pathname>
 */
int parse_list_objects_filter(struct list_objects_filter_options *filter_options,
			      const char *arg)
{
	const char *v0;
	const char *v1;

	if (filter_options->choice)
		die(_("multiple object filter types cannot be combined"));

	if (reject_injection_chars(arg))
		die(_("invalid character in filter-spec"));

	filter_options->raw_value = strdup(arg);

	if (skip_prefix(arg, "blob:", &v0)) {

		if (!strcmp(v0, "none")) {
			filter_options->choice = LOFC_BLOB_NONE;
			return 0;
		}

		if (skip_prefix(v0, "limit=", &v1) &&
		    git_parse_ulong(v1, &filter_options->blob_limit_value)) {
			filter_options->choice = LOFC_BLOB_LIMIT;
			return 0;
		}

	} else if (skip_prefix(arg, "sparse:", &v0)) {

		if (skip_prefix(v0, "oid=", &v1)) {
			struct object_context oc;
			struct object_id sparse_oid;
			filter_options->choice = LOFC_SPARSE_OID;
			if (!get_oid_with_context(v1, GET_OID_BLOB,
						  &sparse_oid, &oc))
				filter_options->sparse_oid_value =
					oiddup(&sparse_oid);
			return 0;
		}

		if (skip_prefix(v0, "path=", &v1)) {
			filter_options->choice = LOFC_SPARSE_PATH;
			filter_options->sparse_path_value = strdup(v1);
			return 0;
		}
	}

	die(_("invalid filter-spec expression '%s'"), arg);
	return 0;
}

int opt_parse_list_objects_filter(const struct option *opt,
				  const char *arg, int unset)
{
	struct list_objects_filter_options *filter_options = opt->value;

	assert(arg);
	assert(!unset);

	return parse_list_objects_filter(filter_options, arg);
}
