import textwrap

indent = ' ' * 4

def fill(text):
    tmp = textwrap.fill(text)
    return textwrap.indent(tmp, indent)


def cpp_array_initializer(arr):
    return '{%s}' % (', '.join(map(str, arr)))
