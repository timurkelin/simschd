classdef and_list_c < handle
   
   properties ( Constant = true, GetAccess = private )
      class_id = 'and_list_c';
   end

   properties ( SetAccess = private, GetAccess = public )
      n_evt;
      n_msk;
   end % properties   
   
   properties ( SetAccess = public, GetAccess = public )
      map;
   end % properties
   
   properties ( Constant = true, GetAccess = public )
      map_free    = 0;
      map_match   = 1;
      map_mapped  = 2;
   end
   
   properties ( Constant = true, GetAccess = private )         
      cmap = [1 1 1; ...
              0 0 1; ...
              1 0 0];
   end   
   
   methods
      % Class constructor   
      function this = and_list_c( prm )
         assert( isscalar( prm ), [this.class_id, '(): Incorrect parameter specification.']);   
         
         this.n_evt = prm.n_evt;
         this.n_msk = prm.n_msk;
         
         this.map = ones( this.n_msk, this.n_evt ) .* this.map_free;
      end % function
      
      function done = run( this )
         % create map vectors
         evt_msk_cnt = sum( this.map == this.map_match, 1 ); % how many masks match each event
         msk_evt_cnt = sum( this.map == this.map_match, 2 ); % how many events match each mask
         
         msk_evt_map = zeros( this.n_msk, 1 );
         evt_msk_cnt( evt_msk_cnt == 0 ) = 2 * this.n_msk + 1;
         
         if( ~any( msk_evt_cnt == 0 )) 
            while( true )
               % Find mask with min number of matching events
               [msk_cnt, msk_idx] = min( msk_evt_cnt ); 

               if( msk_cnt <= this.n_evt )
                  % Find event with min number of matching masks
                  evt_vec = this.map( msk_idx, : );
                  [~, evt_idx] = min( evt_msk_cnt + ( evt_vec == this.map_free ) .* ( 2 * this.n_msk ));

                  % Save msk-event mapping
                  msk_evt_map( msk_idx ) = evt_idx;

                  % Update map metrics
                  msk_evt_cnt( msk_idx ) = 2 * this.n_evt + 1; 
                  evt_msk_cnt( evt_idx ) = 2 * this.n_msk + 1;

                  msk_evt_cnt = msk_evt_cnt - this.map( :, evt_idx ); 
                  evt_msk_cnt = evt_msk_cnt - this.map( msk_idx, : ); 

                  evt_msk_cnt( evt_msk_cnt == 0 ) = 2 * this.n_msk + 1;
                  msk_evt_cnt( msk_evt_cnt == 0 ) = 2 * this.n_evt + 1;                
               else
                  break;
               end
            end
         end
         
         done = true;
         for msk_idx = 1:this.n_msk
            if( msk_evt_map( msk_idx ))
               assert( this.map( msk_idx, msk_evt_map( msk_idx )) == this.map_match )
               this.map( msk_idx, msk_evt_map( msk_idx )) = this.map_mapped;
            else
               done = false;
            end
         end
         
         assert( all( sum( this.map == this.map_mapped, 1 ) <= 1 )); % each mask is mapped to 1 event
         assert( all( sum( this.map == this.map_mapped, 2 ) <= 1 )); % each event corresponds to 1 mask at max
         
      end % function
      
      function hnd = plot( this, done )
         xval = cell( 1, this.n_evt );
         yval = cell( 1, this.n_msk );
         
         for e_idx = 1:this.n_evt
            xval{e_idx} = sprintf( 'e%02d', e_idx );
         end
         
         for m_idx = 1:this.n_msk
            yval{m_idx} = sprintf( 'm%02d', m_idx );
         end   
         
         hnd = heatmap( xval, yval, this.map );
         
         hnd.Colormap    = this.cmap;
         hnd.ColorLimits = [this.map_free, ... 
                            this.map_mapped];
                         
         if( done )
            hnd.Title = 'Mapped OK';
         else
            hnd.Title = 'Unmapped';
         end
         
      end % function
   end % methods
   
end % classdef
