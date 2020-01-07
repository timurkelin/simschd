
prm = struct( 'n_evt', 20, ... 
              'n_msk', 10 );
           
and_list = and_list_c( prm );

if( false )
   rng( 'shuffle' );

   match_frac = 0.18;

   match = ones( numel( and_list.map ), 1 ) .* and_list.map_free;

   match( 1:round( numel( and_list.map ) * match_frac )) = 1;

   and_list.map( randperm( numel( and_list.map ))) = ( match ~= 0 ) .* and_list.map_match;
end

if( false )
   and_list.map( :, 1 ) = and_list.map_match;
end

if( false )
   and_list.map = eye( size( and_list.map ));
end

if( true )
   rng( 'shuffle' );

   match_frac = 0.07;

   match = ones( numel( and_list.map ), 1 ) .* and_list.map_free;

   match( 1:round( numel( and_list.map ) * match_frac )) = 1;
   
   and_list.map( randperm( numel( and_list.map ))) = match;
   and_list.map = mod( and_list.map + eye( size( and_list.map )), 2 );
   
   and_list.map = ( and_list.map ~= 0 )  .* and_list.map_match;
end

done = and_list.run();

and_list.plot( done );
