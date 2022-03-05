#!/usr/bin/env python3

import sys
import re
import colored

style_bold = colored.attr('bold')
style_reset = colored.attr('reset')

style_head = colored.attr('bold')
style_true = colored.fg('light_green') + style_bold
style_false = colored.fg('light_red') + style_bold

regex_head = re.compile(r'(\s*\w.+)*')
regex_content = re.compile(
    r'(?P<input>(\s*\w.+[^\w]?)*)(?P<osep>\s*[^\w]\s*)(?P<output>\w.*)')

match = regex_head.search(sys.stdin.readline())
print(f"{style_head}{match.group(0)}{style_reset}")
for line in sys.stdin:
    matches = regex_content.search(line)

    result = int(matches['output']) != 0
    print(
        f"{matches['input']}{matches['osep']}{style_true if result else style_false}{matches['output']}{style_reset}")

sys.exit()
