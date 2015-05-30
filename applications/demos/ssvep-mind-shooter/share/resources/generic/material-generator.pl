#!/usr/bin/perl

$name = $ARGV[0];

sub writeMaterial($$$)
{
	my ($material, $dark, $angle) = @_;

	$dark = $dark ? "-dark" : '';
	my $suffix = $angle ? "-$angle" : '';

	$effect = "";
	$effect = "rotate $angle" if $angle;

	return qq[

	material GenericSurface/$material$suffix$dark
	{
		technique
		{
			pass
			{
				ambient 1 1 1 1
				diffuse 1 1 1 1
				specular 0 0 0 0
				emissive 0 0 0

				scene_blend alpha_blend

				texture_unit
				{
					texture $material$dark.png
					$effect
				}
			}
		}
	}

];
}

opendir DIR, "textures";
@files = readdir DIR;
closedir DIR;

@files = grep (/(?<!-dark)\.png$/, @files);

foreach $texture (@files)
{
	$texture =~ s/\.png//;
	
	print writeMaterial($texture, 0, 0);
	print writeMaterial($texture, 1, 0);
	print writeMaterial($texture, 0, 90);
	print writeMaterial($texture, 1, 90);
	print writeMaterial($texture, 0, 180);
	print writeMaterial($texture, 1, 180);
}
