#!/bin/bash
#
# Generate markdown ChangeLog files based on git history.

tmp=$(mktemp)
touch ${tmp}

# Tagging has been rather ad-hoc in the repo thus far.
# The 0.1-x tags are ignored as they live on a branch
TAGS="paflib-0.1 "

sane_tags=$(git tag -l [0-9]*.[0-9]*.[0-9]* | sort -V)
sorted_tags="${TAGS} ${sane_tags}"

prev_tag=""
echo "# ChangeLog"
for tag in ${sorted_tags}; do
	# Prepend tags to the output because we want the ChangeLog in the
	# reverse order, e.g. 2.0.0, 1.0.1 and 1.0.0.
	{
		date=$(git show --date=short --format='tformat:%cd' -s ${tag})
		echo -e "\n## ${tag} - ${date}\n"
		git --no-pager log --no-merges --format='format: - %s%n' \
		    ${prev_tag:+${prev_tag}..}${tag} | cat - ${tmp}
	} > ${tmp}.1
	mv ${tmp}.1 ${tmp}
	prev_tag=${tag}
done

# Send the output to stdout.
cat ${tmp}

rm ${tmp}
