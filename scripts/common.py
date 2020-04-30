import textwrap

indent = ' ' * 4

def fill(text):
    tmp = textwrap.fill(text)
    return textwrap.indent(tmp, indent)


def cpp_array_initializer(arr):
    return '{%s}' % (', '.join(map(str, arr)))

def compose2(f, g):
    return lambda x: f(g(x))

def cpp_arrayarray_initializer(arr):
    return '{%s}' % (',\n '.join(map(compose2(fill,cpp_array_initializer), arr)))
