import operator
import random
import ast

from tqdm import tqdm
import pytest

import symint


@pytest.mark.parametrize(
    'args, kwargs, expected_range',
    [
        [(-42, 42), {                        }, (-42 , 42  )],
        [(-42,   ), {                        }, (-42 , None)],
        [(       ), {'min': -42              }, (-42 , None)],
        [(       ), {             'max': 42  }, (None, 42  )],
        [(       ), {'min': -42 , 'max': 42  }, (-42 , 42  )],
        [(       ), {'min': None, 'max': 42  }, (None, 42  )],
        [(       ), {'min': -42 , 'max': None}, (-42 , None)],
        [(       ), {'min': None, 'max': None}, (None, None)],
    ]
)
def test_symint_construction(args, kwargs, expected_range):
    s = symint.SymInt()
    assert isinstance(s, symint.SymInt)
    assert isinstance(s.range, tuple)
    assert s.range == (None, None)

    s = symint.SymInt(42)
    assert isinstance(s, symint.SymInt)
    assert s.range == (42, 42)

    s = symint.SymInt('i', *args, **kwargs)
    assert isinstance(s, symint.SymInt)
    assert s.range == expected_range


def test_symint_int_cast():
    s = symint.SymInt(42)
    assert int(s) == 42

    s = symint.SymInt()
    with pytest.raises(RuntimeError):
        int(s)

    s = symint.SymInt('i', min = 42, max = 42)
    assert int(s) == 42


class TestUnaryOps:
    @pytest.mark.parametrize('op', [operator.pos, operator.neg])
    @pytest.mark.parametrize('x_int', [42, -42])
    def test_unary_ops(self, op, x_int):
        x = symint.SymInt(x_int)
        assert int(op(x)) == op(x_int)

        x = symint.SymInt()
        assert op(x).range == (None, None)

    def test_pos(self):
        x = symint.SymInt('i', 7, 42)
        r = +x
        assert r.range == (7, 42)

    def test_neg(self):
        x = symint.SymInt('i', 7, 42)
        r = -x
        assert r.range == (-42, -7)


@pytest.mark.parametrize('op', [operator.add, operator.sub, operator.mul, operator.floordiv, operator.mod])
@pytest.mark.parametrize('x_int', [42, -42])
@pytest.mark.parametrize('y_int', [7, -7])
def test_binary_ops(op, x_int, y_int):
    x = symint.SymInt(x_int)
    y = symint.SymInt(y_int)
    assert int(op(x, y)) == op(x_int, y_int)
    assert int(op(x, y_int)) == op(x_int, y_int)
    assert int(op(x_int, y)) == op(x_int, y_int)
    assert op(x, y).range == (op(x_int, y_int),)*2

    x = symint.SymInt()
    y = symint.SymInt()
    assert op(x, y).range == (None, None)
    assert op(y, x).range == (None, None)


def test_mul_ranges():
    i = symint.SymInt('i', min = -10, max = 100)
    j = symint.SymInt('j', min = -100, max = 10)
    assert (i * j).range == (-10000, 1000)


@pytest.mark.parametrize(
    'i_lims, j_lims, expected_range',
    [
        [(   0,  100), ( 8, 10), ( 0, 9)],
        [(-100,  100), ( 8, 10), (-9, 9)],
        [(None, None), ( 8, 10), (-9, 9)],
        [(None, None), ( 1,  1), ( 0, 0)],
        [(   5,  100), (10, 10), ( 0, 9)],
        [(   5,  100), ( 8, 10), ( 0, 9)],
    ],
)
def test_c_mod_ranges(i_lims, j_lims, expected_range):
    i = symint.SymInt('i', *i_lims)
    j = symint.SymInt('j', *j_lims)
    assert symint.trunc_mod(i, j).range == expected_range


@pytest.mark.parametrize(
    'i_lims, j_lims, expected_range',
    [
        [(   0,  100), ( 8, 10), ( 0, 9)],
        [(-100,  100), (-8, 10), (-7, 9)],
        [(None, None), ( 8, 10), ( 0, 9)],
    ],
)
def test_mod_ranges(i_lims, j_lims, expected_range):
    i = symint.SymInt('i', *i_lims)
    j = symint.SymInt('j', *j_lims)
    assert (i % j).range == expected_range


def test_large_trees():
    i = symint.SymInt('i')
    j = symint.SymInt('j', min = -75)
    k = symint.SymInt('k', max = 789)
    l = symint.SymInt('l', min = -1234, max = 12345)
    vars = [i, j, k, l]
    ops = (operator.add, operator.mul, operator.floordiv, operator.mod)
    for _ in tqdm(range(int(1e6))):  # stack overflow
        var_x = random.choice(vars)
        var_y = random.choice(vars)
        op = random.choice(ops)
        i = random.randrange(len(vars))
        vars[i] = op(var_x, var_y)



# --------------------------------------------------------



def test_addition():
    i = symint.SymInt('i')
    j = symint.SymInt('j', min = -10, max = 10)
    k = symint.SymInt('k', min = -10, max = 10)

    assert (i + j).range == (None, None)
    assert (j + k).range == (-20, 20)


def test_multiplication():
    x = symint.SymInt(2)
    y = symint.SymInt(42)
    z = symint.SymInt('i')

    assert int(x * y) == 84
    assert isinstance(y * z, symint.SymInt)


def test_division():
    x = symint.SymInt(2)
    y = symint.SymInt(42)
    z = symint.SymInt('i')

    assert int(y / x) == 21
    assert isinstance(y / z, symint.SymInt)


def test_conversion_to_string():
    x = symint.SymInt(42)
    y = symint.SymInt()
    z = symint.SymInt('i')
    assert repr(x + y + z) == '(42 + ? + i)'
    assert str(x*y + z) == '((42*?) + i)'
    assert str(x/y) == '(42/?)'


@pytest.mark.parametrize('new_range', [(-5, 5), (3, 3), (None, None), (None, 5), (-5, None)])
def test_set_range(new_range):
    i = symint.SymInt('i')
    i.range = new_range
    assert i.range == new_range


def test_set_range_exceptions():
    i = symint.SymInt('i', min = -100, max = 100)
    j = 2*i

    with pytest.raises(RuntimeError):
        j.range = (-5, 5)

    assert i.range == (-100, 100)
    assert j.range == (-200, 200)

    with pytest.raises(RuntimeError):
        j.range = (-100, 101)

    assert i.range == (-100, 100)

    with pytest.raises(RuntimeError):
        j.range = (-101, 100)

    assert i.range == (-100, 100)


def test_recompute_basic():
    i = symint.SymInt('i')
    j = i
    for _ in range(10**1):
        j = -j

    j = (j + j - j) * i
    print(j)
    i.range = 5, 5
    print(j)
    assert repr(j) == '25'
    assert int(j) == 25


def test_division_and_modulus_simplification_for_positives():
    i = symint.SymInt('i')
    j = symint.SymInt('j')
    assert str(i // j) == '(i//j)'
    assert str(i % j) == '(i %% j)'

    i = symint.SymInt('i', min = 0)
    j = symint.SymInt('j', min = 0)
    assert str(i // j) == '(i/j)'
    assert str(i % j) == '(i % j)'


def test_double_negation_eliminated():
    i = symint.SymInt('i', min=-10, max=10)
    assert str(-(-i)) == 'i'
    assert (-(-i)).range == (-10, 10)


def test_self_subtraction_is_zero():
    i = symint.SymInt('i', min=-5, max=5)
    result = i - i
    assert int(result) == 0
    assert result.range == (0, 0)


def test_subtract_negation_becomes_addition():
    i = symint.SymInt('i', min=1, max=10)
    j = symint.SymInt('j', min=1, max=10)
    result = i - (-j)
    assert result.range == (2, 20)
    assert str(result) == '(i + j)'


def test_multiply_by_minus_one():
    i = symint.SymInt('i', min=1, max=10)
    assert str(symint.SymInt(-1) * i) == '-(i)'
    assert str(i * symint.SymInt(-1)) == '-(i)'
    assert (symint.SymInt(-1) * i).range == (-10, -1)


def test_divide_by_minus_one():
    i = symint.SymInt('i', min=2, max=10)
    result = i / symint.SymInt(-1)
    assert str(result) == '-(i)'
    assert result.range == (-10, -2)


def test_floor_div_by_minus_one():
    i = symint.SymInt('i', min=2, max=10)
    result = i // symint.SymInt(-1)
    assert str(result) == '-(i)'
    assert result.range == (-10, -2)


def test_modulus_by_one_is_zero():
    i = symint.SymInt('i', min=-100, max=100)
    # Both floor_mod(i, 1) and c_mod(i, 1) should simplify to 0
    assert int(i % symint.SymInt(1)) == 0          # floor_mod
    assert int(symint.trunc_mod(i, symint.SymInt(1))) == 0  # truncating mod


def test_modulus_by_minus_one_is_zero():
    i = symint.SymInt('i', min=-100, max=100)
    # c_mod(i, -1) should simplify to 0
    assert int(symint.trunc_mod(i, symint.SymInt(-1))) == 0


def test_floor_mod_by_minus_one_is_zero():
    i = symint.SymInt('i', min=-100, max=100)
    # floor_mod(i, -1) == i % -1 should simplify to 0
    assert int(i % symint.SymInt(-1)) == 0


class TestExpressionRendering:
    def test_large_trees(self):
        i = symint.SymInt('i')
        j = symint.SymInt('j')
        k = symint.SymInt('k')
        l = symint.SymInt('l')
        vars = (i, j, k, l)
        ops = (operator.add, operator.mul, operator.floordiv, operator.mod)
        expr = random.choice(vars)
        for _ in range(int(80)):
            var = random.choice(vars)
            expr = random.choice(ops)(expr, var)

        expr_str = str(expr)
        expr_c_code = expr.c_code()
        assert len(str(expr_str)) > 0
        assert len(str(expr_c_code)) > 0

    def test_deep_trees(self):
        i = symint.SymInt('i')
        expr = i
        for _ in range(int(1e6)):
            expr = expr + i
            expr = expr * i

        expr_str = str(expr)
        expr_c_code = expr.c_code()
        assert len(str(expr_str)) > 0
        assert len(str(expr_c_code)) > 0

    def test_ano_c_code(self):
        i = symint.SymInt('i')
        j = symint.SymInt('j')
        ano = symint.SymInt()

        with pytest.raises(RuntimeError):
            print(ano.c_code())

        with pytest.raises(RuntimeError):
            print((ano + 1).c_code())

        with pytest.raises(RuntimeError):
            print((i + j + ano).c_code())

    @pytest.mark.parametrize('func,expected', [
        (lambda i, j, _: i + j, '(i + j)'),
        (lambda i, j, k: i * j, '(i*j)'),
        (lambda i, j, k: i % j, '(((i % j) + j) % j)'),
        (lambda i, j, k: i // j, '((i - (((i % j) + j) % j)) / j)'),
        (lambda i, j, k: (i - j)*(j + k), '((i - j)*(j + k))')
    ])
    def test_specific_cases(self, func, expected):
        i = symint.SymInt('i')
        j = symint.SymInt('j')
        k = symint.SymInt('k')

        expr = func(i, j, k).c_code()
        assert expr == expected
        i, j, k = 3.14, 2.7, 1.6
        assert eval(expr)

    def test_specific_cases_positives(self):
        i = symint.SymInt('i', min = 0)
        j = symint.SymInt('j', min = 0)
        k = symint.SymInt('k', min = 0)

        assert str(i % j) == '(i % j)'
        assert (i % j).c_code() == '(i % j)'

        assert str(i + j + k) == '(i + j + k)'
        assert (i + j + k).c_code() == '(i + j + k)'

        assert str((i + j + k) * (i + j + k)) == '((i + j + k)*(i + j + k))'
        assert symint.trunc_div(i - j, j + k).c_code() == '((i - j)/(j + k))'


class TestConfig:
    def setup_method(self):
        symint.config.set_print_as_c_code(False)

    def teardown_method(self):
        symint.config.set_print_as_c_code(False)

    def test_default_is_false(self):
        assert symint.config.get_print_as_c_code() is False

    def test_set_true(self):
        symint.config.set_print_as_c_code(True)
        assert symint.config.get_print_as_c_code() is True

    def test_set_false(self):
        symint.config.set_print_as_c_code(True)
        symint.config.set_print_as_c_code(False)
        assert symint.config.get_print_as_c_code() is False
