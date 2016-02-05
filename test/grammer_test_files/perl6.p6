use v6; # tell Perl 6 (and tooling!) that we're a Perl 6 file

# our first program; you may recognize say from Perl 5.10
say 'Hello, World!';

# variable declarations are just like Perl 5!
my $name = 'Rob';

# ...only you can have an optional type (ex. $full-name = 3 would fail)
# and you can have hyphens in the variable name
my Str $full-name = 'Rob Hoelz';

# concat isn't . anymore, it's now ~
# . is the method call operator
say 'Hello, ' ~ $name;

# the Perlish way, of course, is to simply interpolate
say "Hello, $name!";

# you can use { } in a string to interpolate any arbitrary
# code
say "Hello, { $name.subst(/R/, 'B') }!";

# method calls using () are also interpolated by default, so
# the { } above are redundant
say "Hello, $full-name.subst(/R/, 'B')!";

# arrays are more or less the same
my @values = 1, 3, 5;

# only sigils don't vary - $values[0] is illegal
say @values[1];

# arrays don't interpolate like they used to...
say "values = @values";

# ...but a single value of an array does!
say "values = @values[1]";

# you can interpolate the entire array with empty square brackets
# at the end
say "values = @values[]";

# ...or just use the explicit interpolation block
say "values = {@values}";

# hashes have undergone similar changes to arrays
my %hash = name => 'Rob', :talking-about<Perl6>; # Hashes are built from Pairs, and these are two alternative syntaxes for Pairs

say %hash{'name'}; # curly braces don't autoquote anymore

say %hash<talking-about>; # use angle braces for autoquoting

# the WHAT method asks an object its type (there are other interrogative methods)
say $name.WHAT;

# you can stringify an object in a few ways...
$name.Str;  # basic "convert me to a string" form (also ~$name)
$name.gist; # human friendly output
$name.perl; # Perl-friendly output (you should be able to EVAL this)

# subroutines have signatures now, with optional types and return value
# specifications
sub is-even(Int $n) returns Bool { $n %% 2 }

# multi dispatch is handy
multi fib(0) { 0 }
multi fib(1) { 1 }
multi fib(Int $n where * >= 0 ) { fib($n - 2) + fib($n - 1) }

# classes are easy!
class Color {
    # has declares an attribute
    has $.r is rw = 0;
    has $.g is rw = 0;
    has $.b is rw = 0;

    # methods are simple too
    method hex { '0x' ~ ($.r, $.g, $.b).map: -> $value { sprintf('%x', $value) } }
}

my $blue = Color.new(:b(0xff)); # a class constructor takes Pairs of its attributes to initialize them
my $red  = Color.new(r => 0xff);

# here's a rudimentary calculator grammar!
# In Perl 6, we have "regexes", which are a whole different beast from
# the regular expressions you're used to.  You'll see some familiar
# faces, some faux amis, and some brand new constructs, but Perl 6
# regexes are one of my favorite Perl 6 features because they're so
# powerful
grammar CalculatorParser {
    rule TOP { # the top rule is where the parse begins
        ^ <expr> $ # we match a single expression within a string using the expr subrule, which gets captured into the expr capture group
    }

    rule expr { # this is the rule TOP was referring to!
        $<operand1>=<value> # we capture a value using the value subrule and stick it into the operand1 capture group
        <operator>          # capture an operator
        $<operand2>=<value> # similar to above
    }

    token value {
        \d+ # just a sequence of one or more digits
    }

    token operator { # one of the basic arithmetic operators; || is alternation instead of |; | is now longest matching alternation
        '+' || '-' || '*' || '/'
    }
}

# ...and its corresponding actions class
# actions classes are a way of working on the AST that a grammar is 
# building as it's being built.  You can alter the AST that rules higher
# up are seeing by using make(); think of it as a special kind of return
# for action methods.
class CalculatorActions {
    method operator($/) { # here, we'll start bottom-up.  $/ is the default match variable, which
                          # allows to access captures using $<...> shorthand.
        given ~$/ { # dispatch over the Str form of the match
            when '+' {
                make &infix:<+> # &infix:<...> is syntactic sugar for the Code object
                                # that implements the infix operator between the <...>
                                # by make()'ing it, we pass it up to the action method
                                # that invoked the corresponding rule in the grammar;
                                # in this case, expr
            }
            when '-' {
                make &infix:<->
            }
            when '*' {
                make &infix:<*>
            }
            when '/' {
                make &infix:</>
            }
        }
    }

    method value($/) {
        make +$/; # just make the AST of this the Num value of the match
                  # (that's what prefix + does)
    }

    method expr($/) {
        my $op = $<operator>.made; # get the thing that the operator action method
                                   # called make() on; ie. the Code object that implements
                                   # the operator

        make $op($<operand1>.made, $<operand2>.made); # now call the operator on its operands
    }

    method TOP($/) {
        make $<expr>.made; # just get the value of the expression!
    }
}

say CalculatorParser.parse('3 + 7', :actions(CalculatorActions)).made; # prints 10