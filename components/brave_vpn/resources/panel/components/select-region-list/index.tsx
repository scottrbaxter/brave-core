import * as React from 'react'
import * as S from './style'
import { Button, Radio } from 'brave-ui'
import { CaratStrongLeftIcon } from 'brave-ui/components/icons'
import { Region } from '../../api/region_interface'
import locale from '../../constants/locale'
interface Props {
  onDone: Function
  onRegionClick: Function
  regions: Array<Region>
  selectedRegion: Region
}

function SelectRegion (props: Props) {
  const handleGoBackClick = () => {
    props.onDone()
  }

  const handleItemClick = (currentRegion: Region) => {
    props.onRegionClick(currentRegion)
  }

  return (
    <S.Box>
      <S.PanelContent>
        <S.PanelHeader>
          <S.BackButton
            type='button'
            onClick={handleGoBackClick}
          >
            <CaratStrongLeftIcon />
          </S.BackButton>
        </S.PanelHeader>
        <S.RegionList>
          <Radio
            value={{ [props.selectedRegion.name]: true }}
            size={'small'}
            disabled={false}
          >
            {props.regions.map((entry: Region, i: number) => (
              <div
                key={i}
                data-value={entry.name}
              >
                <S.RegionLabel
                  onClick={handleItemClick.bind(this, entry)}
                >
                  {entry.namePretty}
                </S.RegionLabel>
              </div>
            ))}
          </Radio>
        </S.RegionList>
        <S.ActionArea>
          <Button
            level='primary'
            type='accent'
            brand='rewards'
            text={locale.connectLabel}
          />
        </S.ActionArea>
      </S.PanelContent>
    </S.Box>
  )
}

export default SelectRegion
